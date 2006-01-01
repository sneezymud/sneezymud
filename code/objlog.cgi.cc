#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include <string>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;


int main(int argc, char **argv){
  Cgicc cgi;
  // TDatabase db(DB_SNEEZYPROD);
  TDatabase db(DB_SNEEZYBETA);
  toggleInfo.loadToggles();

  form_iterator name = cgi.getElement("name");
  form_iterator vnum = cgi.getElement("vnum");
  form_iterator start = cgi.getElement("start");

  if (start == cgi.getElements().end()) {
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;

    cout << "<form method=post action=\"/angus/objlog.cgi\">" << endl;
    cout << "Object Name: <input type=text name=name><br>" << endl;
    cout << "Object VNum: <input type=text name=vnum><br>" << endl;
    cout << "<input type=hidden name=start value=1" << ">";
    cout << "<input type=submit><br>" << endl;
    cout << body() << endl;
  } else {
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;

    cout << "<form method=post action=\"/angus/objlog.cgi\">" << endl;
    cout << "Object Name: <input type=text name=name><br>" << endl;
    cout << "Object VNum: <input type=text name=vnum><br>" << endl;
    cout << "<input type=hidden name=start value=1" << ">";
    cout << "<input type=submit><br>" << endl;
    if ((**vnum).empty() && (**name).empty()) {
      db.query("select l.vnum, o.name, l.loadtime, l.objcount from objlog l, obj o where l.vnum = o.vnum order by l.loadtime");
    } else if ((**vnum).empty()) {
      db.query("select l.vnum, o.name, l.loadtime, l.objcount from objlog l, obj o where l.vnum = o.vnum and o.name like '%s' order by l.loadtime", (**name).c_str());
    } else if ((**name).empty()) {
      db.query("select l.vnum, o.name, l.loadtime, l.objcount from objlog l, obj o where l.vnum = o.vnum and l.vnum=%i order by l.loadtime", convertTo<int>(**vnum));
    }
    cout << "<table border=1>" << endl;
    cout << "  <tr>" << endl;
    cout << "    <th align center>VNum</th>" << endl;
    cout << "    <th align center>Object Name</th>" << endl;
    cout << "    <th align center>Loadtime</th>" << endl;
    cout << "    <th align center>Count</th>" << endl;
    cout << "  </tr>" << endl;

    while(db.fetchRow()){
      cout << "  <tr valign=top>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["vnum"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["name"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["loadtime"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["objcount"]) << "</td>" << endl;
      cout << "  </tr>" << endl;
    }
    cout << "</table>" << endl;
    cout << body() << endl;
  }
  cout << html() << endl;
}
