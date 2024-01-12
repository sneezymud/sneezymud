//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      spec_rooms.cc : special procedures for rooms
//
///////////////////////////////////////////////////////////////////////////

// cmd = CMD_ROOM_ENTERED
//      if ch dies, leave ch valid and return DELETE_THIS
//
// cmd = CMD_GENERIC_PULSE
//
//  cmd = normal
//      return DELETE_VICT to kill ch

#include <boost/format.hpp>
#include <ext/alloc_traits.h>
#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ansi.h"
#include "being.h"
#include "comm.h"
#include "connect.h"
#include "db.h"
#include "defs.h"
#include "enum.h"
#include "gametime.h"
#include "handler.h"
#include "limbs.h"
#include "log.h"
#include "low.h"
#include "monster.h"
#include "obj.h"
#include "obj_portal.h"
#include "person.h"
#include "race.h"
#include "room.h"
#include "spec_rooms.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"
#include "thing.h"
#include "toggle.h"

int oft_frequented_room(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TBeing* mob;
  int i, q;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  switch (rp->number) {
    case Room::KINDRED_INN:
    case Room::GREEN_DRAGON_INN:
      // enter between 7PM and 10PM
      if ((GameTime::getHours() >= 19) && (GameTime::getHours() <= 22)) {
        q = ::number(1, 2);
        for (i = 1; i <= q; i++) {
          if (::number(0, 10))
            continue;
          int rmob =
            real_mobile(::number(0, 1) ? Mob::MALE_HOPPER : Mob::FEMALE_HOPPER);
          int maxMob = mob_index[rmob].max_exist;
          if (mob_index[rmob].getNumber() >= maxMob)
            continue;
          mob = read_mobile(rmob, REAL);
          *rp += *mob;
          *rp << *mob;
          act(
            "$n saunters into the bar anticipating $s first ale of the "
            "evening.",
            false, mob, nullptr, nullptr, TO_ROOM);
        }
      }
      break;
    case Room::PEW1:
    case Room::PEW2:
    case Room::PEW3:
    case Room::PEW4:
      if (!((GameTime::getDay() + 1) % 7) &&  // on Sunday
          (GameTime::getHours() == 10)) {     // at 10
        for (i = 1; i <= 8; i++) {
          if (::number(0, 9))
            continue;
          int rmob = real_mobile(
            ::number(0, 1) ? Mob::MALE_CHURCH_GOER : Mob::FEMALE_CHURCH_GOER);
          int maxMob = mob_index[rmob].max_exist;
          if (mob_index[rmob].getNumber() >= maxMob)
            continue;
          mob = read_mobile(rmob, REAL);
          *rp += *mob;
          act("$n quietly sits down on the pew here for church.", false, mob,
            nullptr, nullptr, TO_ROOM);
        }
      }
      break;
    case Room::TOP_OF_TREE:
      // april 4th at noon
      if ((GameTime::getMonth() == 3) && (GameTime::getDay() == 3) &&
          (GameTime::getHours() == 12)) {
        int rom = real_mobile(Mob::SONGBIRD);
        if (mob_index[rom].getNumber() > 100)
          break;
        for (i = 1; i <= 200; i++) {
          mob = read_mobile(rom, REAL);
          *rp += *mob;
        }
        act("It's a once-a-year event! And you're here to witness it!", false,
          mob, nullptr, nullptr, TO_ROOM);
        act(
          "Two hundred songbirds descend from the skies and land on the big "
          "tree!",
          false, mob, nullptr, nullptr, TO_ROOM);
      }
      break;
    default:
      vlogf(LOG_PROC,
        format("Room %d has an oft_frequented_room() with no code for it.") %
          rp->number);
      break;
  }
  return false;
}

int prisonDump(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp) {
  TThing* t;
  TRoom* roomp;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (!(roomp = real_roomp(31904))) {
    vlogf(LOG_BUG, "couldn't find sewage pipe in prisonDump!");
    return false;
  }

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);

    // Only objs get nuked
    TObj* obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;

    // portals should not be nuked
    if (dynamic_cast<TPortal*>(obj))
      continue;

    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;

    // nor should flares
    if (obj->objVnum() == Obj::GENERIC_FLARE)
      continue;

    sendrpf(rp, "A %s slides down the chute into the disposal pipe below.\n\r",
      fname(obj->name).c_str());

    --(*obj);
    *roomp += *obj;
  }
  return false;
}

// this is a crappy proc but will work in a pinch
int personalHouse(TBeing* ch, cmdTypeT cmd, const char*, TRoom* rp) {
  char buf[80];
  TPerson* pers;

  if ((cmd != CMD_RENT) && (cmd != CMD_SNAP))
    return false;

  pers = dynamic_cast<TPerson*>(ch);
  if (!pers)
    return false;

  sprintf(buf, "%s", fname(rp->name).c_str());

  if (buf != pers->getName())
    return false;

  pers->sendTo("You snap your fingers with authority.\n\r");
  act("$n snaps $s fingers with authority.", true, pers, 0, 0, TO_ROOM);

  pers->sendTo("Your house swallows you whole.\n\r");
  act("Uh oh, $n's house just swallowed $m.", true, pers, 0, 0, TO_ROOM);

  pers->cls();

  return pers->saveRent(true, 2);
}

int Whirlpool(TBeing* ch, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  TRoom* rp2;
  int new_room;
  int rc;

  if (cmd == CMD_GENERIC_PULSE) {
    if (rp->stuff.empty())
      return false;

    // transport stuff out of here
    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      TBeing* tch = dynamic_cast<TBeing*>(t);
      if (tch && tch->isImmortal())
        continue;

      for (new_room = ::number(12500, 13100);;
           new_room = ::number(12500, 13100)) {
        if (!(rp2 = real_roomp(new_room)) || !(rp2->isWaterSector()))
          continue;
        break;
      }
      --(*t);
      *rp2 += *t;
    }

    return false;
  }

  if (cmd != CMD_ROOM_ENTERED)
    return false;

  if (ch->isImmortal())
    return false;

  // normal damage is 0 - 25% max hp
  int dam = ::number(0, ch->hitLimit() / 4);

  // 50% chance of extra damage
  if ((ch->getRace() == RACE_DWARF) || (::number(1, 100) <= 50)) {
    act("The whirlpool tears $n limb from limb before spitting you out.", true,
      ch, 0, 0, TO_ROOM);
    ch->sendTo(
      "The whirlpool tears you limb from limb before spitting you out.\n\r");

    // double damage, ie 0-05% max hp
    dam *= 2;
  } else {
    act("$n miraculously manages to swim out of the whirlpool's clutches.",
      true, ch, 0, 0, TO_ROOM);
    ch->sendTo(
      "Miraculously, you somehow manage to escape the whirlpool's "
      "clutches!\n\r");
    ch->sendTo(
      "You desparately swim for the surface with your lungs burning for a "
      "breath!\n\r");
  }

  if (ch->riding) {
    rc = ch->fallOffMount(ch->riding, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
  }
  while ((t = ch->rider)) {
    rc = t->fallOffMount(ch, POSITION_STANDING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = nullptr;
    }
  }

  --(*ch);

  for (new_room = ::number(12500, 13100);; new_room = ::number(12500, 13100)) {
    if (!(rp2 = real_roomp(new_room)) || !(rp2->isWaterSector()))
      continue;
    break;
  }
  *rp2 += *ch;

  // do the damage
  if (ch->reconcileDamage(ch, dam, DAMAGE_WHIRLPOOL) == -1)
    return DELETE_THIS;

  return true;
}

bool isBelimusAlive(void) {
  return mob_index[real_mobile(Mob::BELIMUS)].getNumber() >= 1;
}

int belimusThroat(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  TBeing* ch = nullptr;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0, 9))
    return false;

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);
    ch = dynamic_cast<TBeing*>(t);
    if (!ch)
      continue;

    if (ch->isImmortal())
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo(
        "Belimus has expired and the weight of his corpse collapses upon "
        "you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = nullptr;
      continue;
    }

    ch->sendTo("Belimus's throat muscles constrict slightly.\n\r");

    int dam = ::number(3, 5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_SUFFOCATION) == -1) {
      delete ch;
      ch = nullptr;
    }
  }

  if (rp->number == 13480) {
    TRoom* rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_NORTH];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_SOUTH];
    if (IS_SET(exitp->condition, EXIT_CLOSED)) {
      if (!::number(0, 60)) {
        sendrpf(rp, "A rumbling is heard as the windpipe opens.\n\r");
        sendrpf(rp2, "A rumbling is heard as the windpipe opens.\n\r");
        REMOVE_BIT(exitp->condition, EXIT_CLOSED);
        REMOVE_BIT(back->condition, EXIT_CLOSED);
      }
    } else {
      if (!::number(0, 20)) {
        sendrpf(rp, "A rumbling is heard as the windpipe closes.\n\r");
        sendrpf(rp2, "A rumbling is heard as the windpipe closes.\n\r");
        SET_BIT(exitp->condition, EXIT_CLOSED);
        SET_BIT(back->condition, EXIT_CLOSED);
      }
    }
  }
  return true;
}

int belimusStomach(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  TBeing* ch = nullptr;
  int rc;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0, 9))
    return false;

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);
    ch = dynamic_cast<TBeing*>(t);
    if (!ch)
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo(
        "Belimus has expired and the weight of his corpse collapses upon "
        "you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = nullptr;
      continue;
    }

    if (ch->isImmortal())
      continue;

    ch->sendTo("The acid in Belimus's stomach corrodes you.\n\r");
    rc = ch->acidEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete ch;
      ch = nullptr;
      continue;
    }

    int dam = ::number(3, 5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_ACID) == -1) {
      delete ch;
      ch = nullptr;
      continue;
    }
  }
  if (rp->number == 13490) {
    TRoom* rp2;
    roomDirData* exitp;
    exitp = rp->dir_option[DIR_SOUTH];
    rp2 = real_roomp(exitp->to_room);
    if (IS_SET(exitp->condition, EXIT_CLOSED)) {
      if (!::number(0, 60)) {
        sendrpf(rp, "A rumbling is heard as the intestines distend.\n\r");
        sendrpf(rp2, "A rumbling is heard as the intestines distend.\n\r");
        REMOVE_BIT(exitp->condition, EXIT_CLOSED);
      }
    } else {
      if (!::number(0, 20)) {
        sendrpf(rp, "A rumbling is heard as the intestines contract.\n\r");
        sendrpf(rp2, "A rumbling is heard as the intestines contract.\n\r");
        SET_BIT(exitp->condition, EXIT_CLOSED);
      }
    }
  }
  return true;
}

int belimusLungs(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  TBeing* ch = nullptr;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0, 9))
    return false;

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);
    ch = dynamic_cast<TBeing*>(t);
    if (!ch)
      continue;

    if (!isBelimusAlive()) {
      ch->sendTo(
        "Belimus has expired and the weight of his corpse collapses upon "
        "you!!\n\r");
      ch->sendTo("Man, sucks to be buried in blubber...\n\r");
      ch->rawKill(DAMAGE_NORMAL);
      delete ch;
      ch = nullptr;
      continue;
    }

    if (ch->isImmortal())
      continue;

    ch->sendTo("Belimus inhales and you are buffeted by the wind.\n\r");

    int dam = ::number(3, 5);
    if (ch->reconcileDamage(ch, dam, DAMAGE_GUST) == -1) {
      delete ch;
      ch = nullptr;
    }
  }
  if (rp->number == 13496) {
    TRoom* rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_WEST];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_EAST];
    if (IS_SET(exitp->condition, EXIT_CLOSED)) {
      if (!::number(0, 60)) {
        sendrpf(rp, "A rumbling is heard as Belimus inhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus inhales.\n\r");
        REMOVE_BIT(exitp->condition, EXIT_CLOSED);
        REMOVE_BIT(back->condition, EXIT_CLOSED);
      }
    } else {
      if (!::number(0, 20)) {
        sendrpf(rp, "A rumbling is heard as Belimus exhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus exhales.\n\r");
        SET_BIT(exitp->condition, EXIT_CLOSED);
        SET_BIT(back->condition, EXIT_CLOSED);
      }
    }
  }
  if (rp->number == 13498) {
    TRoom* rp2;
    roomDirData *exitp, *back;
    exitp = rp->dir_option[DIR_EAST];
    rp2 = real_roomp(exitp->to_room);
    back = rp2->dir_option[DIR_WEST];
    if (IS_SET(exitp->condition, EXIT_CLOSED)) {
      if (!::number(0, 60)) {
        sendrpf(rp, "A rumbling is heard as Belimus inhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus inhales.\n\r");
        REMOVE_BIT(exitp->condition, EXIT_CLOSED);
        REMOVE_BIT(back->condition, EXIT_CLOSED);
      }
    } else {
      if (!::number(0, 20)) {
        sendrpf(rp, "A rumbling is heard as Belimus exhales.\n\r");
        sendrpf(rp2, "A rumbling is heard as Belimus exhales.\n\r");
        SET_BIT(exitp->condition, EXIT_CLOSED);
        SET_BIT(back->condition, EXIT_CLOSED);
      }
    }
  }
  return true;
}

int belimusBlowHole(TBeing* me, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  TBeing *ch = nullptr, *mob;

  if ((cmd != CMD_UP) && ((cmd != CMD_GENERIC_PULSE) || ::number(0, 9)))
    return false;

  if (cmd == CMD_GENERIC_PULSE) {
    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      ch = dynamic_cast<TBeing*>(t);
      if (!ch)
        continue;

      if (!isBelimusAlive()) {
        ch->sendTo(
          "Belimus has expired and the weight of his corpse collapses upon "
          "you!!\n\r");
        ch->sendTo("Man, sucks to be buried in blubber...\n\r");
        ch->rawKill(DAMAGE_NORMAL);
        delete ch;
        ch = nullptr;
        continue;
      }
    }
    return true;
  }

  if (me && (cmd == CMD_UP)) {
    roomDirData* exitp;
    TRoom* rp2;

    if (!(exitp = rp->dir_option[DIR_UP]))
      return false;
    if (IS_SET(exitp->condition, EXIT_CLOSED))
      return false;

    for (mob = character_list; mob; mob = mob->next) {
      if (mob->mobVnum() == Mob::BELIMUS) {
        int room2 = mob->in_room;
        rp2 = real_roomp(room2);
        for (int i = 0; i <= 9; i++)
          if ((exitp = rp2->dir_option[i]) && (room2 = exitp->to_room))
            break;

        --(*me);
        thing_to_room(me, room2);
        act("$n is ejected from Belimus's blowhole.", 0, me, 0, 0, TO_ROOM);
        me->sendTo(
          "You are ejected from the blowhole and land somewhere nearby.\n\r");
        return true;
      }
    }
    return false;
  }

  return true;
}

void findMostExpensiveItem(TObj* item, TObj*& best) {
  if (!item)
    return;
  for (StuffIter it = item->stuff.begin(); it != item->stuff.end(); ++it)
    findMostExpensiveItem(dynamic_cast<TObj*>(*it), best);
  if (best && best->obj_flags.cost >= item->obj_flags.cost)
    return;
  best = item;
}

TObj* findMostExpensiveItem(TBeing* vict) {
  TObj* best = nullptr;
  for (int i = MIN_WEAR; i < MAX_WEAR; i++)
    if (vict->equipment[i])
      findMostExpensiveItem(dynamic_cast<TObj*>(vict->equipment[i]), best);
  for (StuffIter it = vict->stuff.begin(); it != vict->stuff.end(); ++it)
    findMostExpensiveItem(dynamic_cast<TObj*>(*it), best);
  return best;
}

int weirdCircle(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp) {
  TBeing* mob;

  if (cmd == CMD_ENTER) {
    sstring buf;
    one_argument(arg, buf);
    if (!is_abbrev(buf, "circle"))
      return false;

    int mobnum = real_mobile(17111);
    if (mob_index[mobnum].getNumber())
      return false;  // already loaded
    if (!(mob = read_mobile(mobnum, REAL))) {
      vlogf(LOG_PROC, "bad load of mob in weirdCircle");
      return false;
    }

    *rp += *mob;
    act("$n steps into the circle from nowhere.", true, mob, 0, nullptr, TO_ROOM);
    mob->doSay(
      "Welcome!  It's been some time since anyone's come to visit me.");
    act("$n looks in your direction.", true, mob, 0, ch, TO_VICT);
    act("$n looks in $N's direction.", true, mob, 0, ch, TO_NOTVICT);

    TObj* obj;
    if (!(obj = findMostExpensiveItem(ch))) {
      act("$n frowns and turns $s back on you.", true, mob, 0, ch, TO_VICT);
      act("$n frowns and turns away from $N.", true, mob, 0, ch, TO_NOTVICT);
      return true;
    }

    mob->doSay("I see you've brought me a gift!");
    mob->doSay("Allow me to relieve you of your burden.");
    act("$n claps $s hands together once sharply.", true, mob, 0, nullptr,
      TO_ROOM);

    if (obj->parent)
      --(*obj);
    else if (obj->eq_pos > WEAR_NOWHERE)
      ch->unequip(obj->eq_pos);
    *mob += *obj;
    return true;

    // stone oracle hangs around on avg 60 secs
  } else if (cmd == CMD_GENERIC_PULSE && !::number(0, 600)) {
    int mobnum = real_mobile(17111);
    if (mobnum < 0) {
      vlogf(LOG_PROC, "Bogus mob specified in weirdCircle.");
      return false;
    }

    if (!mob_index[mobnum].getNumber() ||
        !(mob = get_char_room(mob_index[mobnum].name, rp->number)))
      return false;

    if (mob->fight())
      return false;

    if (mob->stuff.size())
      mob->doAction("", CMD_BOW);
    mob->doSay("Now, please leave me to my slumber.");
    act("$n fades quickly from the circle.", true, mob, 0, nullptr, TO_ROOM);

    for (StuffIter it = mob->stuff.begin(); it != mob->stuff.end();) {
      TThing* t = *(it++);
      delete t;
    }
    delete mob;
    return true;
  }

  return false;
}

int noiseBoom(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TRoom* new_room;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0, 99))
    return false;

  sendrpf(rp, "BOOM!!\n\r");

  for (dirTypeT door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] &&
        (new_room = real_roomp(rp->dir_option[door]->to_room)))
      if (new_room != rp)
        sendrpf(new_room, "An eerie moaning sound echoes through the tunnel.");
  }
  return false;
}

// false if didn't slide
// DELETE_THIS on death
int genericSlide(TThing* t, TRoom* rp) {
  TThing *t2, *t3;
  int rc;

  if (t->isLevitating() || t->isFlying())
    return false;

  t->sendTo("You seem to be sliding down the incline!!!\n\r");
  act("$n slides down the incline.", true, t, 0, 0, TO_ROOM);

  for (t2 = t->rider; t2; t2 = t3) {
    t3 = t2->nextRider;
    rc = genericSlide(t2, rp);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t2;
      t2 = nullptr;
    }
  }

  (*t)--;
  switch (rp->number) {
    case 11347:
      thing_to_room(t, 11349);
      break;
    case 11349:
      thing_to_room(t, 11351);
      break;
    case 11351:
      thing_to_room(t, 11353);
      break;
    case 11423:
      thing_to_room(t, 11422);
      break;
    case 20593:
      thing_to_room(t, 20412);
      break;
    case 20594:
      thing_to_room(t, 20593);
      break;
    case 20597:
      thing_to_room(t, 20594);
      break;
    default:
      vlogf(LOG_PROC, format("Bogus room for generic slide %d") % rp->number);
      thing_to_room(t, Room::VOID);
  }

  TBeing* tbt = dynamic_cast<TBeing*>(t);
  if (tbt)
    tbt->doLook("", CMD_LOOK);

  act("$n slides into the room.", true, t, 0, 0, TO_ROOM);

  rc = t->genericMovedIntoRoom(t->roomp, -1);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  // a special case for Mithros's Penguins
  if (tbt && tbt->in_room == 20412) {
    // at end of slide
    if (tbt->mobVnum() == Mob::PENGUIN_ADULT ||
        tbt->mobVnum() == Mob::PENGUIN_YOUNG) {
      act("You quickly dive under the water and swim away.", false, tbt, 0, 0,
        TO_CHAR);
      act("$n quickly dives under the water and swims away.", false, tbt, 0, 0,
        TO_ROOM);

      --(*tbt);
      thing_to_room(tbt, 20597);

      act("You find your way into a new room.", false, tbt, 0, 0, TO_CHAR);
      act("From high above you, $n suddenly slides into the room.", false, tbt,
        0, 0, TO_ROOM);
    }
  }

  return true;
}

int slide(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TThing* t;
  int rc;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);

    if (t == nullptr)
      vlogf(LOG_BUG,
        format("Null pointer in roomproc slide() in room %s") % rp->getName());
    if (t->riding)
      continue;

    if (::number(0, 2))
      return false;

    rc = genericSlide(t, rp);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = nullptr;
    }
  }

  return true;
}

int SecretPortalDoors(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp) {
  char buf[255];
  TObj* portal = nullptr;
  int found = false;
  TRoom* other_room = nullptr;
  TThing* temp = nullptr;
  int found_other = false;

  //  int destination = nullptr;

  if (ch && cmd < MAX_CMD_LIST) {
    if (rp->number != ch->in_room) {
      vlogf(LOG_PROC, format("char %s not in proper room (SecretPortalDoors)") %
                        ch->getName());
      return false;
    }
    one_argument(arg, buf, cElements(buf));
  }

  switch (rp->number) {
    case 7228:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return false;
      if (is_abbrev(buf, "lever") || is_abbrev(buf, "pole")) {
        act("As you pull the lever the lift begins to move with a loud creak.",
          true, ch, 0, 0, TO_CHAR);
        act("As $n pulls the lever the lift begins to move with a loud creak.",
          true, ch, 0, 0, TO_ROOM);
      } else
        return false;
      if (obj_index[real_object(Obj::MINELIFT_DOWN)].getNumber() >= 1) {
        act("Nothing seems to happen.", true, ch, 0, 0, TO_CHAR);
        act("Nothing seems to happen.", true, ch, 0, 0, TO_ROOM);
        return true;
      }

      if (!(portal = read_object(Obj::MINELIFT_DOWN, VIRTUAL))) {
        vlogf(LOG_PROC, format("Problem loading object in SecretPortal. (%d)") %
                          Obj::MINELIFT_DOWN);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return false;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("With a loud boom the entry platform is dropped as the lift arrives.",
        true, ch, portal, 0, TO_CHAR);
      act("With a loud boom the entry platform is dropped as the lift arrives.",
        true, ch, portal, 0, TO_ROOM);
      if (obj_index[real_object(Obj::MINELIFT_UP)].getNumber() >= 1) {
        other_room = real_roomp(7266);
        temp = nullptr;
        found_other = false;
        if (other_room) {
          for (StuffIter it = other_room->stuff.begin();
               it != other_room->stuff.end();) {
            temp = *(it++);
            if (!dynamic_cast<TPortal*>(temp))
              continue;
            if (temp->number == real_object(Obj::MINELIFT_UP)) {
              delete temp;
              temp = nullptr;
              found_other = true;
              break;
            }
          }
          if (found_other) {
            sendToRoom("With a loud boom, the entry platform disappears.\n\r",
              7266);
          }
        }
      }
      return true;
    case 7268:
      if (cmd == CMD_GENERIC_PULSE) {
        // automatic shift change at 6AM and 6PM
        if (GameTime::getHours() != 6 && GameTime::getHours() != 18)
          return false;

        if (obj_index[real_object(7214)].getNumber())
          return true;

        if (!(portal = read_object(7214, VIRTUAL))) {
          vlogf(LOG_PROC,
            format("Problem loading object in SecretPortal. (%d)") % 7214);
          return false;
        }
        thing_to_room(portal, 7268);
        act("The drawbridge is lowered.", false, portal, 0, 0, TO_ROOM);

        // load into other room
        if (!(portal = read_object(7215, VIRTUAL))) {
          vlogf(LOG_PROC,
            format("Problem loading object in SecretPortal. (%d)") % 7215);
          return false;
        }
        thing_to_room(portal, 7265);
        act("The drawbridge is lowered.", false, portal, 0, 0, TO_ROOM);

        return true;
      } else if (cmd == CMD_LOWER) {
        if (!is_abbrev(buf, "bridge") && !is_abbrev(buf, "drawbridge")) {
          return false;
        }
        int rob = real_object(7214);

        if (obj_index[rob].getNumber()) {
          act("The drawbridge is already lowered.", true, ch, 0, 0, TO_CHAR);
          return true;
        }

        act("You lower the drawbridge.", false, ch, 0, 0, TO_CHAR);
        act("$n lowers the drawbridge.", false, ch, 0, 0, TO_ROOM);

        if (!(portal = read_object(rob, REAL))) {
          vlogf(LOG_PROC,
            format("Problem loading object in SecretPortal. (%d)") % 7214);
          return false;
        }
        *ch->roomp += *portal;

        // load into other room
        if (!(portal = read_object(7215, VIRTUAL))) {
          vlogf(LOG_PROC,
            format("Problem loading object in SecretPortal. (%d)") % 7215);
          return false;
        }
        thing_to_room(portal, 7265);
        act("The drawbridge is lowered from the other side.", false, portal, 0,
          0, TO_ROOM);

        return true;
      } else if (cmd == CMD_RAISE) {
        if (!is_abbrev(buf, "bridge") && !is_abbrev(buf, "drawbridge")) {
          return false;
        }
        int rob = real_object(7214);

        if (!obj_index[rob].getNumber()) {
          act("The drawbridge is already raised.", true, ch, 0, 0, TO_CHAR);
          return true;
        }

        act("You raise the drawbridge.", false, ch, 0, 0, TO_CHAR);
        act("$n raises the drawbridge.", false, ch, 0, 0, TO_ROOM);

        // remove it from this room
        for (StuffIter it = ch->roomp->stuff.begin();
             it != ch->roomp->stuff.end();) {
          temp = *(it++);
          if (!dynamic_cast<TPortal*>(temp))
            continue;
          if (temp->number == rob) {
            delete temp;
            temp = nullptr;
            break;
          }
        }
        // remove it from other room
        rob = real_object(7215);
        for (StuffIter it = real_roomp(7265)->stuff.begin();
             it != real_roomp(7265)->stuff.end();) {
          temp = *(it++);
          if (!dynamic_cast<TPortal*>(temp))
            continue;
          if (temp->number == rob) {
            act("$n is raised from the other side.", false, temp, 0, 0,
              TO_ROOM);
            delete temp;
            temp = nullptr;
            break;
          }
        }
        return true;
      }
      return false;
    case 7266:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return false;
      if (is_abbrev(buf, "lever") || is_abbrev(buf, "pole")) {
        act("As you pull the lever the lift begins to move with a loud creak.",
          true, ch, 0, 0, TO_CHAR);
        act("As $n pulls the lever the lift begins to move with a loud creak.",
          true, ch, 0, 0, TO_ROOM);
      } else
        return false;
      if (obj_index[real_object(Obj::MINELIFT_UP)].getNumber() >= 1) {
        act("Nothing seems to happen.", true, ch, 0, 0, TO_CHAR);
        act("Nothing seems to happen.", true, ch, 0, 0, TO_ROOM);
        return true;
      }

      if (!(portal = read_object(Obj::MINELIFT_UP, VIRTUAL))) {
        vlogf(LOG_PROC, format("Problem loading object in SecretPortal. (%d)") %
                          Obj::MINELIFT_DOWN);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return false;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("With a loud boom the entry platform is dropped as the lift arrives.",
        true, ch, portal, 0, TO_CHAR);
      act("With a loud boom the entry platform is dropped as the lift arrives.",
        true, ch, portal, 0, TO_ROOM);
      if (obj_index[real_object(Obj::MINELIFT_DOWN)].getNumber() >= 1) {
        other_room = real_roomp(7228);
        temp = nullptr;
        found_other = false;
        if (other_room) {
          for (StuffIter it = other_room->stuff.begin();
               it != other_room->stuff.end();) {
            temp = *(it++);
            if (!dynamic_cast<TPortal*>(temp))
              continue;
            if (temp->number == real_object(Obj::MINELIFT_DOWN)) {
              delete temp;
              temp = nullptr;
              found_other = true;
              break;
            }
          }
          if (found_other) {
            sendToRoom("With a loud boom, the entry platform disappears.\n\r",
              7228);
          }
        }
      }

      return true;
    case 15277:
      if (cmd != CMD_PULL && cmd != CMD_TUG)
        return false;
      for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end(); ++it) {
        TObj* io = dynamic_cast<TObj*>(*it);
        if (io && io->objVnum() == Room::TREE_BRIDGE) {
          portal = io;
          found = true;
        }
      }

      if (is_abbrev(buf, "lever")) {
        act("You pull the lever.", true, ch, 0, 0, TO_CHAR);
        act("$n pulls on a lever.", true, ch, 0, 0, TO_ROOM);

        // deleting the portal
        if (found) {
          act("As you pull on the lever you hear a noise from the bridge.",
            true, ch, 0, 0, TO_CHAR);
          act(
            "The bridge rises until it looks just like one of the surrounding "
            "trees.",
            true, ch, 0, 0, TO_CHAR);
          act("As $n pulls on the lever you hear a noise from the bridge.",
            true, ch, 0, 0, TO_ROOM);
          act(
            "The bridge rises until it looks just like one of the surrounding "
            "trees.",
            true, ch, 0, 0, TO_ROOM);
          delete portal;
          portal = nullptr;
          return true;
        } else {
          // loading the portal
          act("As you pull on the lever some rope releases from above you.",
            true, ch, 0, 0, TO_CHAR);
          act("One of the trees descends to make a bridge across the river.",
            true, ch, 0, 0, TO_CHAR);
          act("As $n pull on the lever some rope releases from above.", true,
            ch, 0, 0, TO_ROOM);
          act("One of the trees descends to make a bridge across the river.",
            true, ch, 0, 0, TO_ROOM);
          portal = read_object(Room::TREE_BRIDGE, VIRTUAL);
          *ch->roomp += *portal;
          return true;
        }
      }
      break;
    case 16173:
      if (cmd != CMD_PUSH && cmd != CMD_PRESS)
        return false;
      if (is_abbrev(buf, "button")) {
        act("You push the button.", true, ch, 0, 0, TO_CHAR);
        act("$n pushes the button.", true, ch, 0, 0, TO_ROOM);
      } else
        return false;
      if (obj_index[real_object(Obj::FLAMING_PORTAL)].getNumber() >= 1) {
        act("Nothing seems to happen.", true, ch, 0, 0, TO_CHAR);
        act("Nothing seems to happen.", true, ch, 0, 0, TO_ROOM);
        return true;
      }

      if (!(portal = read_object(Obj::FLAMING_PORTAL, VIRTUAL))) {
        vlogf(LOG_PROC, format("Problem loading object in SecretPortal. (%d)") %
                          Obj::FLAMING_PORTAL);
        ch->sendTo("Serious problem, contact a god.\n\r");
        return false;
      }
      *ch->roomp += *portal;
      // loading the portal
      act("$p shimmers into view.", true, ch, portal, 0, TO_CHAR);
      act("$p shimmers into view.", true, ch, portal, 0, TO_ROOM);
      return true;
    default:
      return false;
  }
  return false;
}

int getRandomRoom() {
  int to_room;
  TRoom* rp;
  int breakout = 0;

  for (;;) {
    // this keeps getting caught in a loop on builder mud
    // and I don't want to fix it properly.
    if (++breakout > 1000000) {  // presumably we won't ever have > 1 mil rooms
      vlogf(LOG_BUG, "getRandomRoom got caught in a loop");
      return false;
    }

    // note, all rooms below 100 are ignored

    to_room = ::number(100, top_of_world);

    if (!(rp = real_roomp(to_room)))
      continue;
    if (rp->isRoomFlag(ROOM_PRIVATE))
      continue;
    if (rp->isRoomFlag(ROOM_HAVE_TO_WALK))
      continue;
    if (rp->isFlyingSector())
      continue;
    if (zone_table[rp->getZoneNum()].enabled == false)
      continue;

    break;
  }
  return to_room;
}

int randomMobDistribution(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  int exitrnum = 0, rc;
  TMonster* tm;

  static int pulse;
  ++pulse;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  // loop through all directions
  for (dirTypeT d = DIR_NORTH; d < MAX_DIR; d++) {
    if (d == DIR_NORTHEAST || d == DIR_NORTHWEST || d == DIR_SOUTHEAST ||
        d == DIR_SOUTHWEST)
      continue;

    // remove the old exit, if any
    delete rp->dir_option[d];

    // choose a random location
    if (!(exitrnum = getRandomRoom()))
      continue;

    // create the exit
    if (!(rp->dir_option[d] = new roomDirData()))
      continue;

    // link the exit
    rp->dir_option[d]->to_room = exitrnum;
  }

  // this zone is inactive, so let's force a wanderAround
  if (zone_table[rp->getZoneNum()].zone_value == 1) {
    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      TThing* t = *(it++);  // just for safety

      if ((tm = dynamic_cast<TMonster*>(t))) {
        rc = tm->mobileActivity(pulse);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tm;
          tm = nullptr;
        }
      }
    }
  }

  return true;
}

int theKnot(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  static bool done[25];
  int n = rp->number - 2375, exitrnum = 0;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (n < 0 || n >= 25) {
    vlogf(LOG_PROC, format("theKnot: %i is outside [0, 25)") % n);
    return false;
  }

  if (done[n] && ::number(0, 9999))
    return false;

  // loop through all directions
  for (dirTypeT d = DIR_NORTH; d < MAX_DIR; d++) {
    if (d == DIR_NORTHEAST || d == DIR_NORTHWEST || d == DIR_SOUTHEAST ||
        d == DIR_SOUTHWEST)
      continue;

    // if we're redoing our exits and this is an out of zone exit, delete it
    if (rp->dir_option[d] && rp->dir_option[d]->to_room < 2375 &&
        rp->dir_option[d]->to_room > 2399)
      delete rp->dir_option[d];

    // if no exit, chance to add new one
    if (!rp->dir_option[d] && !::number(0, 9)) {
      if (!(exitrnum = getRandomRoom()))
        continue;

      if (!(rp->dir_option[d] = new roomDirData()))
        continue;

      rp->dir_option[d]->to_room = exitrnum;
    }
  }

  //  vlogf(LOG_PEEL, format("the knot: did exits for room %i") %  rp->number);

  done[n] = true;
  return true;
}

int duergarWater(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  if (cmd != CMD_GENERIC_PULSE || !rp)
    return false;

  if (rp->number >= 13755 && rp->number <= 13771) {
    switch ((GameTime::getHours()) % 12) {
      case 0:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        if (rp->getSectorType() != SECT_TEMPERATE_UNDERWATER) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
        }
        break;
      case 7:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      case 8:
      case 9:
      case 10:
      case 11:
        if (rp->getSectorType() != SECT_TEMPERATE_CAVE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_CAVE);
        }
        break;
    }
  } else if (rp->number >= 13738 && rp->number <= 13754) {
    switch ((GameTime::getHours()) % 12) {
      case 1:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      case 2:
      case 3:
      case 4:
      case 5:
        if (rp->getSectorType() != SECT_TEMPERATE_UNDERWATER) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
        }
        break;
      case 6:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 0:
        if (rp->number < 13740 &&
            rp->getSectorType() != SECT_TEMPERATE_ATMOSPHERE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
        } else if (rp->number >= 13740 &&
                   rp->getSectorType() != SECT_TEMPERATE_CAVE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_CAVE);
        }
        break;
    }
    return false;
  } else if (rp->number == 13773) {
    switch ((GameTime::getHours()) % 12) {
      case 2:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      default:
        if (rp->getSectorType() != SECT_TEMPERATE_ATMOSPHERE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
        }
        break;
    }
    return false;
  } else if (rp->number == 13772) {
    switch ((GameTime::getHours()) % 12) {
      case 2:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      case 3:
        if (rp->getSectorType() != SECT_TEMPERATE_UNDERWATER) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_UNDERWATER);
        }
        break;
      default:
        if (rp->getSectorType() != SECT_TEMPERATE_ATMOSPHERE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_ATMOSPHERE);
        }
        break;
    }
    return false;
  } else if (rp->number == 13731) {
    switch ((GameTime::getHours()) % 12) {
      case 3:
        if (rp->getSectorType() != SECT_TEMPERATE_RIVER_SURFACE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_RIVER_SURFACE);
        }
        break;
      default:
        if (rp->getSectorType() != SECT_TEMPERATE_CAVE) {
          // send message here
          rp->setSectorType(SECT_TEMPERATE_CAVE);
        }
        break;
    }
    return false;
  }

  return true;
}

int monkQuestProcLand(TBeing* ch, cmdTypeT cmd, const char*, TRoom* rp) {
  if (cmd != CMD_ROOM_ENTERED)
    return false;
  if (!ch || !ch->hasQuestBit(TOG_MONK_GREEN_FALLING))
    return false;
  if (ch->specials.last_direction != DIR_DOWN)
    return false;

  ch->remQuestBit(TOG_MONK_GREEN_FALLING);
  ch->setQuestBit(TOG_MONK_GREEN_FALLEN);
  act(
    "<c>Having successfully witnessed the elephants fall, you are now prepared "
    "to return to your guildmaster.<1>",
    false, ch, nullptr, nullptr, TO_CHAR);
  return true;
}

int monkQuestProcFall(TBeing* ch, cmdTypeT cmd, const char*, TRoom* rp) {
  TMonster* tmon;

  if (cmd != CMD_ROOM_ENTERED)
    return false;
  if (!ch->riding || !(tmon = dynamic_cast<TMonster*>(ch->riding)) ||
      tmon->mobVnum() != Mob::ELEPHANT)
    return false;
  if (!ch->hasQuestBit(TOG_MONK_GREEN_STARTED))
    return false;

  ch->setQuestBit(TOG_MONK_GREEN_FALLING);
  ch->remQuestBit(TOG_MONK_GREEN_STARTED);
  act(
    "<c>You urge your elephant over the precipice and prepare to observe its "
    "motions...<1>",
    false, ch, nullptr, nullptr, TO_CHAR);

  return true;
}

int BankVault(TBeing*, cmdTypeT cmd, const char*, TRoom* roomp) {
  TRoom* rp;
  TBeing* tb;
  int rc;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (::number(0, 100))
    return false;

  // close and lock vault doors
  //  vlogf(LOG_PEEL, "Bank: closing/locking vault doors");

  if (roomp->number == 31780) {
    if (!IS_SET(roomp->dir_option[DIR_WEST]->condition, EXIT_CLOSED)) {
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EXIT_CLOSED);
      roomp->sendTo("The door to the west swings closed.\n\r");
    }
    if (!IS_SET(roomp->dir_option[DIR_WEST]->condition, EXIT_LOCKED)) {
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EXIT_LOCKED);
      roomp->sendTo("The door to the west locks with an audible *click*.\n\r");
    }

    rp = real_roomp(31779);
    if (!IS_SET(rp->dir_option[DIR_EAST]->condition, EXIT_CLOSED)) {
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EXIT_CLOSED);
      rp->sendTo("The door to the east swings closed.\n\r");
    }
    if (!IS_SET(rp->dir_option[DIR_EAST]->condition, EXIT_LOCKED)) {
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EXIT_LOCKED);
      rp->sendTo("The door to the east locks with an audible *click*.\n\r");
    }
  }

  if (roomp->number == 31786) {
    if (!IS_SET(roomp->dir_option[DIR_WEST]->condition, EXIT_CLOSED)) {
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EXIT_CLOSED);
      roomp->sendTo("The door to the west swings closed.\n\r");
    }
    if (!IS_SET(roomp->dir_option[DIR_WEST]->condition, EXIT_LOCKED)) {
      SET_BIT(roomp->dir_option[DIR_WEST]->condition, EXIT_LOCKED);
      roomp->sendTo("The door to the west locks with an audible *click*.\n\r");
    }

    rp = real_roomp(31785);
    if (!IS_SET(rp->dir_option[DIR_EAST]->condition, EXIT_CLOSED)) {
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EXIT_CLOSED);
      rp->sendTo("The door to the east swings closed.\n\r");
    }
    if (!IS_SET(rp->dir_option[DIR_EAST]->condition, EXIT_LOCKED)) {
      SET_BIT(rp->dir_option[DIR_EAST]->condition, EXIT_LOCKED);
      rp->sendTo("The door to the east locks with an audible *click*.\n\r");
    }
  }

  // check for player in this room and poison if so
  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end(); ++it) {
    if ((tb = dynamic_cast<TBeing*>(*it)) && tb->isPc()) {
      tb->sendTo(COLOR_BASIC,
        "<G>Acidic gas shoots out of small holes in the ceiling.<1>\n\r");
      tb->sendTo(COLOR_BASIC,
        "<r>It burns your skin and you choke uncontrollably!<1>\n\r");

      //  vlogf(LOG_PEEL, format("Bank: %s caught in vault") %  tb->getName());

      rc = tb->reconcileDamage(tb, ::number(20, 50), DAMAGE_TRAP_POISON);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tb;
        continue;
      }

      rc = tb->reconcileDamage(tb, ::number(20, 50), DAMAGE_TRAP_ACID);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tb;
        continue;
      }
    }
  }

  return true;
}

int BankMainEntrance(TBeing*, cmdTypeT cmd, const char*, TRoom* roomp) {
  TRoom* rp;
  static unsigned int pulse;
  TBeing *mob, *boss;
  int i = 0;
  bool found = false;
  Descriptor* d;
  int saferooms[] = {31750, 31751, 31756, 31757, 31758, 31759, 31760, 31764,
    31788, 31789, 31790, 31791, 31792, 31793, 31794, 31795, 31796, 31797, 31798,
    31799, -1};

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  ++pulse;
  int r = 1;
  if (!(pulse % 300)) {
    r = ::number(0, 7);
  }

  if (pulse % 300 || r)
    return false;

  // first, let's close entrance doors
  rp = real_roomp(31764);

  if (!IS_SET(rp->dir_option[DIR_NORTH]->condition, EXIT_CLOSED)) {
    rp->sendTo("The door to the north swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EXIT_CLOSED);
  }
  if (!IS_SET(rp->dir_option[DIR_NORTH]->condition, EXIT_LOCKED)) {
    rp->sendTo("The door to the north locks with an audible *click*.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EXIT_LOCKED);
  }
  if (!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EXIT_CLOSED)) {
    rp->sendTo("The door to the south swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EXIT_CLOSED);
  }

  rp = real_roomp(31767);
  if (!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EXIT_CLOSED)) {
    rp->sendTo("The door to the south swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EXIT_CLOSED);
  }
  if (!IS_SET(rp->dir_option[DIR_SOUTH]->condition, EXIT_LOCKED)) {
    rp->sendTo("The door to the south locks with an audible *click*.\n\r");
    SET_BIT(rp->dir_option[DIR_SOUTH]->condition, EXIT_LOCKED);
  }

  rp = real_roomp(31758);
  if (!IS_SET(rp->dir_option[DIR_NORTH]->condition, EXIT_CLOSED)) {
    rp->sendTo("The door to the north swings closed.\n\r");
    SET_BIT(rp->dir_option[DIR_NORTH]->condition, EXIT_CLOSED);
  }

  // now search for people in the zone and smite'em
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected && d->character && d->character->roomp &&
        d->character->roomp->getZoneNum() == rp->getZoneNum() &&
        !d->character->isImmortal()) {
      found = true;

      for (i = 0; saferooms[i] != -1; ++i) {
        if (d->character->in_room == saferooms[i])
          found = false;
      }

      break;
    }
  }

  // this should prevent multiple groups from loading
  // ... unless they kill the group leader of course
  if (found && !mob_index[real_mobile(31759)].getNumber()) {
    vlogf(LOG_PEEL, "Bank: here comes the wrecking crew");
    // QUEST - commented lines are for easier versions of mobs
    rp = real_roomp(31784);

    boss = read_mobile(31759, VIRTUAL);
    //    boss = read_mobile(29218, VIRTUAL);
    *rp += *boss;
    SET_BIT(boss->specials.affectedBy, AFF_GROUP);

    for (i = 0; i < 4; ++i) {
      mob = read_mobile(31753 + ::number(0, 3), VIRTUAL);
      //      mob = read_mobile(29213+::number(0,3), VIRTUAL);
      *rp += *mob;
      boss->addFollower(mob);
      SET_BIT(mob->specials.affectedBy, AFF_GROUP);
    }
  }

  return true;
}

int dayGateRoom(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TObj* to = nullptr;
  TPortal* obj = nullptr;
  TThing* t = nullptr;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  //  vlogf(LOG_DASH, "daygate proc PULSE");
  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);
    if (!(to = dynamic_cast<TObj*>(t)))
      continue;
    if (obj_index[to->getItemIndex()].virt == Obj::ITEM_DAYGATE) {
      found = true;
      break;
    }
  }
  // vlogf(LOG_DASH, format("daygate proc: found: %s") %  found ? "true" :
  // "false"); vlogf(LOG_DASH, format("daygate proc: hmt: %d  suntime: %d") %
  // GameTime::hourminTime() % sunTime(Weather::SUN_TIME_DAY));
  if (GameTime::hourminTime() > 50 || GameTime::hourminTime() < 46) {
    // code to remove gate
    if (found && to) {
      //     vlogf(LOG_DASH, "daygate proc found gate, removing");
      act("<Y>The radiant portal flares up once and is gone.<1>", true, to, 0,
        0, TO_ROOM);
      --(*to);
      delete to;
    }
  } else {
    // code to place gate
    if (!found) {
      //      vlogf(LOG_DASH, "daygate proc didn't find gate, placing");
      if (!(to = read_object(Obj::ITEM_DAYGATE, VIRTUAL))) {
        vlogf(LOG_LOW, "Error loading daygate");
        return false;
      }
      obj = dynamic_cast<TPortal*>(to);
      if (rp->number == 1303)
        obj->setTarget(5700);
      if (rp->number == 5700)
        obj->setTarget(1303);
      obj->setPortalNumCharges(-1);
      obj->setPortalType(10);
      *rp += *to;
      act(
        "<Y>A shimmering portal as bright as the noonday sun suddenly bursts "
        "into existance.<1>",
        true, to, 0, 0, TO_ROOM);
    }
  }

  return true;
}

int moonGateRoom(TBeing*, cmdTypeT cmd, const char*, TRoom* rp) {
  TObj* to = nullptr;
  TPortal* obj = nullptr;
  TThing* t = nullptr;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
    t = *(it++);
    if (!(to = dynamic_cast<TObj*>(t)))
      continue;
    if (obj_index[to->getItemIndex()].virt == Obj::ITEM_MOONGATE) {
      found = true;
      break;
    }
  }
  if (GameTime::hourminTime() > 2 && GameTime::hourminTime() < 94) {
    // code to remove gate
    if (found && to) {
      //     vlogf(LOG_DASH, "moongate proc found moongate, removing");

      act("<k>The dark portal silently disperses into nothingness.<1>", true,
        to, 0, 0, TO_ROOM);
      --(*to);
      delete to;
    }
  } else {
    // code to place gate
    if (!found) {
      if (!(to = read_object(Obj::ITEM_MOONGATE, VIRTUAL))) {
        vlogf(LOG_LOW, "Error loading moongate");
        return false;
      }
      obj = dynamic_cast<TPortal*>(to);
      if (rp->number == 5895)
        obj->setTarget(28800);
      else if (rp->number == 28800)
        obj->setTarget(5895);
      obj->setPortalNumCharges(-1);
      obj->setPortalType(10);
      *rp += *to;
      // vlogf(LOG_DASH, "moongate proc didn't find moongate, placing");

      act("<k>A portal of midnight darkness suddenly shimmers into reality.<1>",
        true, to, 0, 0, TO_ROOM);
    }
  }

  return true;
}

int boulderRoom(TBeing*, cmdTypeT cmd, const char*, TRoom* roomp) {
  TRoom* rp;
  TThing* t = nullptr;
  TObj* rock;
  int found = 0;
  static unsigned int pulse;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  ++pulse;
  if (pulse % 150)
    return false;

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    if (!(rock = dynamic_cast<TObj*>(t)))
      continue;
    if (obj_index[rock->getItemIndex()].virt == Obj::BOULDER_ITEM) {
      found = 1;
      //      vlogf(LOG_LAPSOS, "found assigned to 1");
      break;
    }
  }

  if (found == 1) {
    //    vlogf(LOG_LAPSOS, format("!found so closing exit - found = %d") %
    //    found);
    rp = real_roomp(4189);
    SET_BIT(rp->dir_option[DIR_DOWN]->condition, EXIT_CLOSED);
    rp = real_roomp(4284);
    REMOVE_BIT(rp->dir_option[DIR_UP]->condition, EXIT_CLOSED);
    return true;
  } else {
    //    vlogf(LOG_LAPSOS, format("found so opening exit - found = %d") %
    //    found);
    rp = real_roomp(4189);
    REMOVE_BIT(rp->dir_option[DIR_DOWN]->condition, EXIT_CLOSED);
    rp = real_roomp(4284);
    REMOVE_BIT(rp->dir_option[DIR_UP]->condition, EXIT_CLOSED);
    return true;
  }
}

extern int sleepTagControl(TBeing*, cmdTypeT, const char*, TRoom*);
extern int sleepTagRoom(TBeing*, cmdTypeT, const char*, TRoom*);
extern int bankRoom(TBeing*, cmdTypeT, const char*, TRoom*);
extern int healing_room(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp);
extern int emergency_room(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp);
extern int SecretDoors(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* rp);

int bogusRoomProc(TBeing*, cmdTypeT, const char*, TRoom* rp) {
  vlogf(LOG_PROC,
    format("WARNING: %s is running around with bogus spec proc #%d") %
      rp->getName() % rp->spec);
  return false;
}

int TRoom::checkSpec(TBeing* ch, cmdTypeT cmd, const char* arg, TThing*) {
  if (spec && spec <= NUM_ROOM_SPECIALS)
    return (roomSpecials[spec].proc)(ch, cmd, arg, this);
  return false;
}

TRoomSpecs roomSpecials[NUM_ROOM_SPECIALS + 1] = {
  {false, "UNUSED", bogusRoomProc}, {false, "UNUSED", bogusRoomProc},  // 1
  {false, "Bank Main Entrance", BankMainEntrance},
  {false, "Bank Vault", BankVault}, {false, "Secret Doors", SecretDoors},
  {false, "Secret Portal Doors", SecretPortalDoors},  // 5
  {false, "Whirlpool", Whirlpool}, {false, "Bank Room", bankRoom},
  {false, "belimus Blow Hole", belimusBlowHole},
  {false, "belimus Lungs", belimusLungs},
  {false, "belimus Stomach", belimusStomach},  // 10
  {false, "belimus Throat", belimusThroat},
  {false, "boulder Room", boulderRoom}, {false, "UNUSED", bogusRoomProc},
  {false, "dayGate Room", dayGateRoom},
  {false, "duergar Water", duergarWater},  // 15
  {false, "UNUSED", bogusRoomProc}, {false, "emergency room", emergency_room},
  {false, "UNUSED", bogusRoomProc}, {false, "healing room", healing_room},
  {false, "monk Quest Proc Fall", monkQuestProcFall},  // 20
  {false, "monk Quest Proc Land", monkQuestProcLand},
  {false, "moonGate Room", moonGateRoom}, {false, "noise Boom", noiseBoom},
  {false, "oft frequented room", oft_frequented_room},
  {false, "UNUSED", bogusRoomProc},  // 25
  {false, "prison Dump", prisonDump},
  {false, "random Mob Distribution", randomMobDistribution},
  {false, "sleep Tag Control", sleepTagControl},
  {false, "sleep Tag Room", sleepTagRoom}, {false, "slide", slide},  // 30
  {false, "the Knot", theKnot}, {false, "weird Circle", weirdCircle},
  {false, "last proc", bogusRoomProc}};

// the following procs are unused but preserved here for future interest
/*
int collapsingTunnel(TBeing *ch, cmdTypeT cmd, const char *, TRoom *rp)
{

  if(cmd!=CMD_ROOM_ENTERED)
    return false;

  act("<k>Rubble<1> collapses <k>behind you, blocking the way you came.<1>",
false, ch, nullptr, nullptr, TO_CHAR); int dam = ::number(21,40); if(::number(0,100) >
ch->plotStat(STAT_CURRENT, STAT_DEX, 0, 100, 50)) { act("<k>You're hit by the
falling rocks!  Ouch!<1>", false, ch, nullptr, nullptr, TO_CHAR); } else { act("<k>You
manage to dodge most of the falling rocks!  Whew.<1>", false, ch, nullptr, nullptr,
TO_CHAR); dam -= 20;
  }

  TRoom *rp2 = nullptr;



  if(ch->in_room == 24643) {

    rp2 = real_roomp(24674);

  }
  if(ch->in_room == 24674) {

    rp2 = real_roomp(24643);

  }
  --(*ch);
  *rp2 += *ch;

  if (ch->reconcileDamage(ch, dam, TYPE_CRUSH) == -1) {
    delete ch;
    ch = nullptr;
  }


  return true;
}


int elfForest(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TRoom *new_room;
  TBeing *mob;
  TObj *arrow, *bow;
  char buf[256], capbuf[256];
  TBeing *vict;
  TThing *t;
  int found = false;

  if ((cmd != CMD_GENERIC_PULSE) || ::number(0,1))
   return false;

  for (t = rp->getStuff(), vict = nullptr; t; t = t->nextThing, vict = nullptr) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (vict->isAnimal() || vict->getRace() == RACE_WOODELF ||
        vict->isImmortal())
      continue;
    break;
  }

  if (!vict)
    return false;

  if (!(mob = read_mobile(10113, VIRTUAL))) {
    return false;
  }
  thing_to_room(mob, Room::VOID);   // safety net to prevent nowhere extract
  if (!(bow = read_object(10145, VIRTUAL))) {
    delete mob;
    return false;
  }
  if (!(arrow = read_object(3412, VIRTUAL))) {
    delete mob;
    delete bow;
    return false;
  }
  arrow->obj_flags.decay_time = 3;
  strcpy(capbuf, mob->getName());

  for (dirTypeT door = MIN_DIR; door < MAX_DIR; door++) {
    if (rp->dir_option[door] &&
            (new_room = real_roomp(rp->dir_option[door]->to_room))) {
      if (new_room == rp || ::number(0,2))
        continue;
      found = false;
      for (t = new_room->getStuff(); t && !found; t = t->nextThing) {
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && tbt->isPc() && !tbt->isImmortal())
           found = true;
      }
      if (!found) {
        --(*mob);  // move out of void
        thing_to_room(mob, rp->dir_option[door]->to_room);
        *mob += *arrow;
        *mob += *bow;
        mob->doBload("bow arrow", 0);
        mob->setSkillValue(SKILL_RANGED_SPEC, MAX_SKILL_LEARNEDNESS);

        sprintf(buf, "%s leaps from the trees to the %s and draws %s %s.\n\r",
             cap(capbuf), dirs[door], mob->hshr(), fname(bow->name).c_str());
        sendrpf(rp, buf);

        sprintf(buf, "%s %s 1", fname(vict->name).c_str(), dirs[rev_dir(door)]);
        mob->doShoot(buf, 0);
        sprintf(buf, "%s scrambles back into the brush.\n\r", cap(capbuf));
        sendrpf(rp, buf);
        // vict is possibly invalid here.
        delete bow;
        delete mob;
        return false;
      }
    }
  }
  delete arrow;
  delete bow;
  delete mob;
  return false;
}


int windGustRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TPerson *player;
  int rc;
  TThing *t;
  static unsigned int pulse;

  if(cmd != CMD_GENERIC_PULSE)
    return false;

  // about 10 per second
  ++pulse;
  // we want to check every minute, but have a 90% of triggering over
  // 5 minutes.  thus, 1-(1-x)^5 = 0.90
  // x = .37
  if(pulse%600 || (::number(0,99) >= 37))
    return false;

  sendrpf(COLOR_BASIC, rp,
    "<c>A strong gust of wind swirls into the room kicking up <o>dust<1><c> and
knocking the unwary off-guard.<1>\n\r");


  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
    t=*(it++);

    if(!(player=dynamic_cast<TPerson *>(t)))
      continue;

    if(player->isImmortal())
      continue;

    // pick random direction
    for (int i = 0; i < 20; i++) {
      dirTypeT dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));

      if (player->canGo(dir)){
  act("You are blown out of the room!",
      false, player, 0, 0, TO_CHAR);
  act("$n is blown out of the room!",
      false, player, 0, 0, TO_ROOM);

  --(*player);
  thing_to_room(player, rp->dir_option[dir]->to_room);

  player->doLook("", CMD_LOOK);
  player->addToWait(combatRound(1));
  break;
      }
    }

    act("You land flat on your back.",
  false, player, 0, 0, TO_CHAR);
    act("$n lands flat on $s back!",
  false, player, 0, 0, TO_ROOM);

    rc = player->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS)){
      delete player;
      player = nullptr;
    }
  }

  return true;
}


void TObj::thingDumped(TBeing *ch, int *value)
{
  int val;

  act("$p vanishes in a puff of smoke.", true, ch, this, 0, TO_ROOM);
  act("$p vanishes in a puff of smoke.", true, ch, this, 0, TO_CHAR);

  // this will take struct and current street value into account
  if (!isObjStat(ITEM_NEWBIE)) {
    val = adjPrice();

    *value += min(1000, max(1, (int) (val *
           ch->plotStat(STAT_CURRENT, STAT_KAR, 0.05, 0.25, 0.18))));
  }

  delete this;
}


void TNote::thingDumped(TBeing *ch, int *)
{
  act("$p vanishes in a puff of smoke.", true, ch, this, 0, TO_ROOM);
  act("$p vanishes in a puff of smoke.", true, ch, this, 0, TO_CHAR);

  delete this;
}


int dump(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TThing *t;
  int value = 0;
  int rc;
  bool wasProp = false;

  if (cmd == CMD_GENERIC_PULSE) {
    for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
      t=*(it++);

      // Only objs get nuked
      TObj *obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;

      // portals should not be nuked
      if (dynamic_cast<TPortal *>(obj))
        continue;

      if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
  continue;

      // nor should flares
      if (obj->objVnum() == Obj::GENERIC_FLARE)
        continue;

      // refuse haulers cart
      if(obj->objVnum() == 33270)
  continue;

      sendrpf(rp, "A %s vanishes in a puff of smoke.\n\r",
fname(obj->name).c_str());

      obj->logMe(nullptr, "Dump nuking");

      delete obj;
      obj = nullptr;
    }
    return false;
  } else if (cmd != CMD_DROP)
    return false;

  if (!ch)
    return false;

  rc = ch->doDrop(arg, nullptr);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
    t=*(it++);

    if (isname("[prop]", t->getName()))
      wasProp = true;

    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;


    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;

    t->thingDumped(ch, &value);
    // t may be invalid afterwards
  }
  if (value && !wasProp) {
    ch->sendTo("You are rewarded for outstanding performance.\n\r");
    act("$n has been rewarded for good citizenship.", true, ch, 0, 0, TO_ROOM);

    if (ch->GetMaxLevel() < 3)
      gain_exp(ch, min(0.010, value/1000.0), -1);
    else {
      // take the global income modifier into account, in times of drought, we
      // don't want folks resorting to using the dump to get their money
      value = (int) (value * gold_modifier[GOLD_INCOME].getVal());
      ch->addToMoney(value, GOLD_DUMP);
    }
  }
  return true;
}


int grimhavenDump(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TThing *t;
  TRoom *roomp;

  if(cmd!=CMD_GENERIC_PULSE)
    return false;

  if(!(roomp=real_roomp(18975))){
    vlogf(LOG_BUG, "couldn't find sewage pipe in grimhavenDump!");
    return false;
  }


  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
    t=*(it++);

    // Only objs get nuked
    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    // portals should not be nuked
    if (dynamic_cast<TPortal *>(obj))
      continue;

    if (obj->isObjStat(ITEM_NOJUNK_PLAYER))
      continue;

    // nor should flares
    if (obj->objVnum() == Obj::GENERIC_FLARE)
      continue;

    sendrpf(rp, "A %s slides down the chute into the disposal pipe below.\n\r",
fname(obj->name).c_str());

    --(*obj);
    *roomp += *obj;
  }
  return false;
}


int pools_of_chaos_and_cleansing(TBeing *ch, cmdTypeT cmd, const char *arg,
TRoom *)
{
  int rc;

  if ((cmd != CMD_ENTER) || !ch->awake())
    return false;

  for (; isspace(*arg); arg++);

  if (is_abbrev(arg, "chaos")) {
    ch->sendTo("You slowly dip yourself into the pool of chaos.\n\r");
    act("$n slowly dips $mself into the pool of chaos.",
              true, ch, nullptr, nullptr, TO_ROOM);
    if (ch->isImmortal())
      return true;

    switch (number(0, 10)) {
      case 0:
  ch->sendTo("As you step into the pool, you suddenly feel much weaker.\n\r");
  act("$n shivers as the water in the pool weakens $m.", true, ch, nullptr, nullptr,
TO_ROOM); ch->setHit(1); return true; case 1: ch->sendTo("As you step into the
pool, your mind becomes clouded.\n\r"); act("$n looks disorientated as $e steps
into the pool.", true, ch, nullptr, nullptr, TO_ROOM); ch->setMana(0); return true;
      case 2:
  ch->sendTo("As you step into the pool, you feel your vision disappear.\n\r");
  act("$n looks around blindly as $e steps into the pool.", true, ch, nullptr,
nullptr, TO_ROOM);

        ch->rawBlind(50, 24 * Pulse::UPDATES_PER_MUDHOUR, SAVE_YES);
  return true;
      case 3:
  ch->sendTo("As you step into the pool, you feel yourself magically
moved.\n\r"); act("$n magically disappears as soon as $e steps into the pool.",
true, ch, nullptr, nullptr, TO_ROOM);

        rc = ch->genericTeleport(SILENT_NO);

        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
  return true;
      case 4:
  ch->sendTo("As you step into the pool, you feel totally refreshed!\n\r");
  act("$n looks totally refreshed as $e steps into the pool.", true, ch, nullptr,
nullptr, TO_ROOM); ch->setHit(ch->hitLimit()); return true; case 5: ch->sendTo("As
you step into the pool, many thoughts rush into your head.\n\r"); act("$n looks
a little dizzy as $h steps into the pool.", true, ch, nullptr, nullptr, TO_ROOM);
  ch->setMana(ch->manaLimit());
  return true;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      default:
  return true;
    }
  } else if (is_abbrev(arg, "cleansing")) {
    ch->sendTo("You slowly dip yourself into the pool of cleansing.\n\r");
    act("$n slowly dips $mself into the pool of cleansing.", true, ch, nullptr,
nullptr, TO_ROOM); return true;
  }
  return false;
}


int waterfallRoom(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TObj *to = nullptr;
  TThing *t = nullptr;
  bool found = false;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
    t=*(it++);
    if(!(to=dynamic_cast<TObj *>(t)))
      continue;
    if(obj_index[to->getItemIndex()].virt == Obj::RAINBOW_MIST) {
      found = true;
      break;
    }
  }
  if (Weather::getSunlight() != Weather::SUN_LIGHT ||
      Weather::getSky() != Weather::SKY_CLOUDLESS) {
    // code to remove rainbow
    if (!found || !to)
      return false;
    act("<W>The sunlight no longer reaches the mist, and $p<W> fades from
sight.<1>", true, to, 0, 0, TO_ROOM);
    --(*to);
    delete to;

  } else {
    // code to place rainbow
    if (found || to)
      return false;
    if (!(to = read_object(Obj::RAINBOW_MIST, VIRTUAL))) {
      vlogf(LOG_LOW, "Error loading rainbow mist object");
      return false;
    }
    *rp += *to;
    act("<W>Suddenly, light from the sun strikes the mist, and $p<W> is
formed.<1>", true, to, 0, 0, TO_ROOM);


  }

  return true;
}
*/
