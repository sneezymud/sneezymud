#include "person.h"
#include "database.h"

#include <set>

namespace {
  struct Candidate
  {
    int vnum;
    int x, y, z;
  };

  int x(TRoom* r, dirTypeT dir)
  {
    if (dir == DIR_EAST || dir == DIR_NORTHEAST || dir == DIR_SOUTHEAST)
      return r->getXCoord() + 1;
    if (dir == DIR_WEST || dir == DIR_NORTHWEST || dir == DIR_SOUTHWEST)
      return r->getXCoord() - 1;
    return r->getXCoord();
  }

  int y(TRoom* r, dirTypeT dir)
  {
    if (dir == DIR_NORTH || dir == DIR_NORTHEAST || dir == DIR_NORTHWEST)
      return r->getYCoord() + 1;
    if (dir == DIR_SOUTH || dir == DIR_SOUTHEAST || dir == DIR_SOUTHWEST)
      return r->getYCoord() - 1;
    return r->getYCoord();
  }

  int z(TRoom* r, dirTypeT dir)
  {
    if (dir == DIR_UP)
      return r->getZCoord() + 1;
    if (dir == DIR_DOWN)
      return r->getZCoord() - 1;
    return r->getZCoord();
  }

  std::map<std::tuple<int, int, int>, int> visitedCoords;

  void visit(TPerson const* ch, Candidate const& c, TRoom* r, TDatabase& db)
  {
    if (r->getXCoord() != 0
        && r->getYCoord() != 0
        && r->getZCoord() != 0) {
      ch->sendTo(boost::format("Error: visited %d / %d twice, how'd that happen?\r\n") % r->number % c.vnum);
      return;
    }

    auto existing = visitedCoords.find(std::make_tuple(c.x, c.y, c.z));
    if (existing != visitedCoords.end()) {
      TRoom* oldRoom = real_roomp(existing->second);
      assert(oldRoom);
      ch->sendTo(boost::format("Conflict @ (%d,%d,%d) between %d %s and %d %s")
          % c.x % c.y % c.z
          % r->number % r->name
          % oldRoom->number % oldRoom->name);
      return;
    }

    r->setXCoord(c.x);
    r->setYCoord(c.y);
    r->setZCoord(c.z);
    db.query("update room set x = %i, y = %i, z = %i where vnum = %i",
        c.x, c.y, c.z, c.vnum);
  }
}

void TPerson::doMapRecalc(int startZ) const
{
  TDatabase db(DB_SNEEZY);
  db.query("begin");

  // it'd be polite to extract the BFS code
  std::queue<Candidate> candidates;
  candidates.push({100, 0, 0, startZ});

  std::set<int> visited;

  while (!candidates.empty()) {
    Candidate c = candidates.front();
    candidates.pop();

    if (visited.count(c.vnum) == 1)
      continue;
    visited.insert(c.vnum);

    TRoom* r = real_roomp(c.vnum);
    if (!r)
    {
      sendTo(boost::format("Error: null roomp for %d\n\r") % c.vnum);
      continue;
    }

    visit(this, c, r, db);

    for (auto exitDir = MIN_DIR; exitDir < MAX_DIR; exitDir++) {
      roomDirData* ex = r->exitDir(exitDir);
      if (ex)
      {
        TRoom* exr = real_roomp(ex->to_room);
        if (!exr)
          sendTo(boost::format("Error: null exitp in %d towards %d\n\r") % r->number % exitDir);

        candidates.push({ex->to_room, x(r, exitDir), y(r, exitDir), z(r, exitDir)});
      }
    }
  }
  db.query("commit");
  sendTo("Recalc done.");
}
