#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include "sstring.h"

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
  sstring my_query;
  unsigned int count = 0;
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
    cout << "    <th align center>Object Type</th>" << endl;
    cout << "    <th align center>Object Name</th>" << endl;
    cout << "    <th align center>Loadtime</th>" << endl;
    cout << "    <th align center>Count</th>" << endl;
    cout << "  </tr>" << endl;

    // my_query = "SELECT l.vnum, i.name AS objtype, o.name, l.loadtime::TIMESTAMP(0), l.objcount FROM objlog l, obj o, itemtypes i WHERE l.vnum = o.vnum AND o.\"type\" = i.\"type\"";
    my_query = "SELECT l.vnum, i.name AS objtype, o.name, l.loadtime::TIMESTAMP(0), l.objcount FROM objlog l LEFT OUTER JOIN obj o USING (vnum) LEFT OUTER JOIN itemtypes i USING (\"type\")";
    if ((**vnum).empty() && (**name).empty()) {
      my_query += " ORDER BY l.loadtime";
      db.query(my_query.c_str());
    } else if ((**name).empty()) {
      my_query += " WHERE l.vnum=%i ORDER BY l.loadtime";
      db.query(my_query.c_str(), convertTo<int>(**vnum));
    } else {
      my_query += " WHERE lower(o.name) LIKE lower('%s') ORDER BY l.loadtime";
      db.query(my_query.c_str(), (**name).c_str());
    }
    while(db.fetchRow()){
      count++;
      cout << "  <tr valign=top>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["vnum"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["objtype"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["name"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["loadtime"]) << "</td>" << endl;
      cout << "    <td align=right>" << stripColorCodes(db["objcount"]) << "</td>" << endl;
      cout << "  </tr>" << endl;
    }
    cout << "</table>" << endl;
    cout << fmt("Number of objects queried:  %i") % count;
    cout << body() << endl;
  }
  cout << html() << endl;
}
