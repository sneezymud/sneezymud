#include "cmd_run.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <ctype.h>
#include <iosfwd>
#include <memory>

#include "being.h"
#include "connect.h"
#include "extern.h"
#include "sstring.h"
#include "structs.h"
#include "toggle.h"

using namespace std;

bool parse(string path, deque<pair<int, char>>& res) {
  int n = 1;

  auto add_int = [&n](const int& k) { n = k; };

  auto add_char = [&n, &res](const char& c) {
    if (c != ' ') {
      res.push_back(make_pair(n, c));
      n = 1;
    }
  };

  // validate a bit
  if (path.find("A") != string::npos || path.find("B") != string::npos ||
      path.find("C") != string::npos || path.find("D") != string::npos)
    return false;
  // transform diagonals for straightforward parsing (much easier than proper
  // stateful lookbehind)
  boost::replace_all(path, "ne", "A");
  boost::replace_all(path, "se", "B");
  boost::replace_all(path, "sw", "C");
  boost::replace_all(path, "nw", "D");

  int acc = 0;
  for (char c : path) {
    if (std::string("neswud ABCD").find(c) != std::string::npos) {
      if (acc != 0) {
        add_int(acc);
        acc = 0;
      }
      add_char(c);
    } else if (isdigit(c)) {
      acc *= 10;
      acc += c - '0';
    } else {
      return false;
    }
  }
  return acc == 0;
}

void TBeing::doRun(const sstring& path) {
  deque<pair<int, char>> res;
  if (parse(path, res)) {
    // suppress automap when running to reduce load and spam
    bool automap = false;
    if (desc) {
      automap = IS_SET(desc->autobits, AUTO_MAP);
      REMOVE_BIT(desc->autobits, AUTO_MAP);
    }

    while (!res.empty()) {
      int steps = res.front().first;
      string dir(1, res.front().second);
      if (dir == "A")
        dir = "ne";
      if (dir == "B")
        dir = "se";
      if (dir == "C")
        dir = "sw";
      if (dir == "D")
        dir = "nw";

      for (int i = 0; i < steps; i++) {
        doMove(getDirFromChar(dir));
      }
      res.pop_front();
    }

    if (desc && automap)
      SET_BIT(desc->autobits, AUTO_MAP);
  } else {
    sendTo("Bad run directions. See 'help run'.");
  }
}
