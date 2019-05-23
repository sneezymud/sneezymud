#include "person.h"

#include <set>

namespace {
  bool canPathThroughDoor(roomDirData* ex)
  {
    if (!ex)
      return false;

    if (IS_SET(ex->condition, EXIT_CAVED_IN))
      return false;

    if (IS_SET(ex->condition, EXIT_CLOSED) && (
          IS_SET(ex->condition, EXIT_SECRET) || IS_SET(ex->condition, EXIT_LOCKED)))
      return false;

    return true;
  }

  // colors in TTerrainInfo
  const char* terrainToColor[16] = {
    "<K>", // black 0
    "<r>", // maroon 1
    "<G>", // green 2
    "<d><g>", // olive 3
    "<B>", // navy 4
    "<P>", // purple 5
    "<c>", // teal 6
    "<d><k>", // silver 7
    "<k>", // gray 8
    "<R>", // red 9
    "<G>", // lime 10
    "<y>", // yellow 11
    "<b>", // blue 12
    "<p>", // magenta 13
    "<C>", // cyan 14
    "<w>", // white 15
  };

  std::vector<std::string> colorize(std::vector<std::string> const& in)
  {
    std::vector<std::string> out;
    out.reserve(in.size());

    for (const auto& row : in) {
      std::string outRow;
      outRow.reserve(in.size() * 3);
      for (char cell : row) {
        if (cell < 0) {
          // cf. TRoom::colorRoom, TerrainInfo
          outRow.append(terrainToColor[TerrainInfo[-cell]->color]);
          outRow.push_back('#'); // current value is terrain type
          outRow.append("<z>");
        } else {
          outRow.push_back(cell);
        }
      }
      out.push_back(outRow);
    }

    return out;
  }

  int x(int old, dirTypeT dir)
  {
    if (dir == DIR_EAST || dir == DIR_NORTHEAST || dir == DIR_SOUTHEAST)
      return old + 1;
    if (dir == DIR_WEST || dir == DIR_NORTHWEST || dir == DIR_SOUTHWEST)
      return old - 1;
    return old;
  }

  int y(int old, dirTypeT dir)
  {
    if (dir == DIR_NORTH || dir == DIR_NORTHEAST || dir == DIR_NORTHWEST)
      return old - 1;
    if (dir == DIR_SOUTH || dir == DIR_SOUTHEAST || dir == DIR_SOUTHWEST)
      return old + 1;
    return old;
  }

  char exitSym(char old, dirTypeT dir)
  {
    switch (dir) {
      case DIR_UP:
        return '^';
      case DIR_DOWN:
        return 'v';
      default:;
    }

    switch (old) {
      case ' ':
        {
          switch (dir) {
            case DIR_NORTH:
            case DIR_SOUTH:
              return '|';
            case DIR_EAST:
            case DIR_WEST:
              return '-';
            case DIR_NORTHEAST:
            case DIR_SOUTHWEST:
              return '/';
            case DIR_NORTHWEST:
            case DIR_SOUTHEAST:
              return '\\';
            default:
              return '!';
          };
        }
      case '|':
        {
          switch (dir) {
            case DIR_NORTH:
            case DIR_SOUTH:
              return '|';
            case DIR_EAST:
            case DIR_WEST:
              return '+';
            case DIR_NORTHEAST:
            case DIR_SOUTHWEST:
            case DIR_NORTHWEST:
            case DIR_SOUTHEAST:
              return '*';
            default:
              return '!';
          };
        }
      case '-':
        {
          switch (dir) {
            case DIR_NORTH:
            case DIR_SOUTH:
              return '+';
            case DIR_EAST:
            case DIR_WEST:
              return '-';
            case DIR_NORTHEAST:
            case DIR_SOUTHWEST:
            case DIR_NORTHWEST:
            case DIR_SOUTHEAST:
              return '*';
            default:
              return '!';
          };
        }
      case '/':
        {
          switch (dir) {
            case DIR_NORTH:
            case DIR_SOUTH:
            case DIR_EAST:
            case DIR_WEST:
              return '*';
            case DIR_NORTHEAST:
            case DIR_SOUTHWEST:
              return '/';
            case DIR_NORTHWEST:
            case DIR_SOUTHEAST:
              return 'X';
            default:
              return '!';
          };
        }
      case '\\':
        {
          switch (dir) {
            case DIR_NORTH:
            case DIR_SOUTH:
            case DIR_EAST:
            case DIR_WEST:
              return '*';
            case DIR_NORTHEAST:
            case DIR_SOUTHWEST:
              return 'X';
            case DIR_NORTHWEST:
            case DIR_SOUTHEAST:
              return '\\';
            default:
              return '!';
          };
        }
      case '*':
        return '*';
      default:
        return old;
    }
  }

  void visit(TPerson const* ch, int halfEdge, int radius, std::vector<std::string>& grid, TRoom const& r)
  {
    if (real_roomp(ch->in_room)->getZCoord() != r.getZCoord())
      return;

    int myX = real_roomp(ch->in_room)->getXCoord();
    int myY = real_roomp(ch->in_room)->getYCoord();
    // coords are now relative to the map's center point (ie. player's location)
    int dx = r.getXCoord() - myX;
    int dy = r.getYCoord() - myY;

    if (max(abs(dx), abs(dy)) > radius)
      return;

    ch->sendTo(boost::format("Visiting %d %s @ (%d,%d) rel (%d,%d)\n\r")
        % r.number % r.name % r.getXCoord() % r.getYCoord() % dx % dy);

    // Let's agree that negative values are sector types, positive values are exits.
    char symbol = -r.getSectorType();

    if (dx == 0 && dy == 0)
      symbol = '@';

    // 2 for the doors
    int hereY = -(2 * dy) + halfEdge;
    int hereX = 2 * dx + halfEdge;
    grid.at(hereY).at(hereX) = symbol;

    ///////
    // exits
    for (auto dir = MIN_DIR; dir < MAX_DIR; dir++) {
      roomDirData* ex = r.exitDir(dir);
      if (r.exitDir(dir) && canPathThroughDoor(ex))
      {
        int exitX = x(hereX, dir);
        int exitY = y(hereY, dir);
        char prevSym = grid.at(exitY).at(exitX);
        grid.at(exitY).at(exitX) = exitSym(prevSym, dir);
      }
    }
  };

}

// TODO: replace queue and set with fixed size array with advancing pointer
// (fixed because the size of map is known in advance)
void TPerson::drawMap(const int radius) const
{
  // 2n+1 rows/columns to accommodate radius of n
  // *2 to fit doors, +2 because we also want doors leading out
  size_t edgeLen = (radius * 2 + 1) * 2 + 2;
  size_t halfEdge = edgeLen / 2;

  std::vector<std::string> grid; // There's probably a clever way to initialize it in one line, maybe
  for (int i = edgeLen; i --> 0;)
    grid.emplace_back(edgeLen, ' ');

  struct Candidate
  {
    int vnum;
    int distance;
  };

  std::queue<Candidate> candidates;
  candidates.push({in_room, 0});

  std::set<int> visited;

  while (!candidates.empty()) {
    Candidate c = candidates.front();
    candidates.pop();

    if (c.distance > 5 * radius)
      continue;

    if (visited.count(c.vnum) == 1)
      continue;
    visited.insert(c.vnum);

    TRoom* r = real_roomp(c.vnum);
    if (!r)
    {
      sendTo("Error: null roomp\n\r");
      continue;
    }

    visit(this, halfEdge, radius, grid, *r);

    for (auto exitDir = MIN_DIR; exitDir < MAX_DIR; exitDir++) {
      roomDirData* ex = r->exitDir(exitDir);
      if (canPathThroughDoor(ex)) {
        // hm, doesn't happen anymore
        if (!real_roomp(ex->to_room))
          sendTo(boost::format("Error: room %d has weird exit towards %d\n\r") % c.vnum % exitDir);
        candidates.push({ex->to_room, c.distance + 1});
      }
    }
  }

  for (const auto& row : colorize(grid))
    sendTo(row + "\n\r");
}
