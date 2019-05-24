#include <deque>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "sstring.h"
#include "being.h"
#include "extern.h"


using namespace std;

namespace {
  deque<pair<int, char> > collection;
  int n = 1;

  void add_int(int const& k) {
    n = k;
  }

  void add_char(char const& c) {
    if (c != ' ') {
      collection.push_back(make_pair(n, c));
      n = 1;
    }
  }
};

bool parse(string path, deque<pair<int, char> >& res )
{
  // validate a bit
  if (path.find("A") != string::npos
      || path.find("B") != string::npos
      || path.find("C") != string::npos
      || path.find("D") != string::npos)
    return false;
  // transform diagonals for straightforward parsing (much easier than proper stateful lookbehind)
  boost::replace_all(path, "ne", "A");
  boost::replace_all(path, "se", "B");
  boost::replace_all(path, "sw", "C");
  boost::replace_all(path, "nw", "D");

  string::const_iterator begin = path.begin();
  string::const_iterator end = path.end();
  using boost::spirit::qi::int_;
  using boost::spirit::qi::char_;

  boost::spirit::qi::parse(
    begin, end,
    *(-int_[&add_int] >> char_("neswud ABCD")[&add_char]));

  if (begin == end) {
    res.resize(collection.size());
    copy(collection.begin(), collection.end(), res.begin());
  }

  collection.clear();

  return begin == end;
}

/*
int main() {
  deque<pair<int, char> > res;

  if (parse("neswud1n2e10s11w20u21d1000n", res)) {
    cout << "Good parse:" << endl;
    while (!res.empty()) {
      cout << res.front().first << res.front().second << endl;
      res.pop_front();
    }
  } else {
    cout << "Bad parse" << endl;
  }
*/

void TBeing::doRun(sstring const& path) {
  deque<pair<int, char> > res;
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
      if (dir == "A") dir = "ne";
      if (dir == "B") dir = "se";
      if (dir == "C") dir = "sw";
      if (dir == "D") dir = "nw";

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
