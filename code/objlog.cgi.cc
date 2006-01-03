#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include <string>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;


void print_form()
{
  cout << "<form method=post action=\"/low/objlog.cgi\">" << endl;
  cout << "Filter on Object Name: <input type=text name=name><br>" << endl;
  cout << "Filter on Object VNum: <input type=text name=vnum><br>" << endl;
  cout << "<input type=hidden name=start value=1" << ">";
  cout << "<input type=submit><br>" << endl;
  cout << "Notes:<br>" << endl;
  cout << "1.  Leaving both fields blank will list all entries in the log.<br>" << endl;
  cout << "2.  The object name field should use the % wildcard character.  Example: %shield%<br>" << endl;
  cout << "3.  If both fields are filled in, the script ignores the VNum field.<br>" << endl;
}

int main(int argc, char **argv)
{
  Cgicc cgi;
  TDatabase db(DB_SNEEZYPROD);
  // TDatabase db(DB_SNEEZYBETA);
  toggleInfo.loadToggles();

  form_iterator name = cgi.getElement("name");
  form_iterator vnum = cgi.getElement("vnum");
  form_iterator start = cgi.getElement("start");

  if (start == cgi.getElements().end()) {
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;
    print_form();
    cout << body() << endl;
  } else {
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;

    print_form();
    cout << "<table border=1>" << endl;
    cout << "  <tr>" << endl;
    cout << "    <th align center>VNum</th>" << endl;
    cout << "    <th align center>Object Name</th>" << endl;
    cout << "    <th align center>Loadtime</th>" << endl;
    cout << "    <th align center>Count</th>" << endl;
    cout << "  </tr>" << endl;

    if ((**vnum).empty() && (**name).empty()) {
      db.query("select l.vnum, o.name, l.loadtime::timestamp(0), l.objcount from objlog l, obj o where l.vnum = o.vnum order by l.loadtime");
    } else if ((**vnum).empty()) {
      db.query("select l.vnum, o.name, l.loadtime::timestamp(0), l.objcount from objlog l, obj o where l.vnum = o.vnum and o.name like '%s' order by l.loadtime", (**name).c_str());
    } else if ((**name).empty()) {
      db.query("select l.vnum, o.name, l.loadtime::timestamp(0), l.objcount from objlog l, obj o where l.vnum = o.vnum and l.vnum=%i order by l.loadtime", convertTo<int>(**vnum));
    }
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
