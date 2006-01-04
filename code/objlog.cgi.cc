#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;

TDatabase db(DB_SNEEZYPROD);
// TDatabase db(DB_SNEEZYBETA);
Cgicc cgi;

form_iterator name = cgi.getElement("name");
form_iterator object_type = cgi.getElement("object_type");
form_iterator vnum = cgi.getElement("vnum");
form_iterator start = cgi.getElement("start");

void print_form()
{
  cout << "<form method=post action=\"/low/objlog.cgi\">" << endl;
  cout << "Filter on Object Name: <input type=text name=name><br>" << endl;
  cout << "Filter on Object Type: <select name=object_type>" << endl;

  db.query("select substr(lower(i.name), 6) as object_type from itemtypes i order by i.name");
  
  cout << "<option value=ALL" << endl;
  cout << "> ALL" << endl;
  while(db.fetchRow()){
    cout << "<option value=" << db["object_type"];
    cout << "> " << db["object_type"] << endl;
  }
  cout << "</select><br>" << endl;
  cout << "Filter on Object VNum: <input type=text name=vnum><br>" << endl;
  cout << "<input type=hidden name=start value=1>" << endl;
  cout << "<input type=submit><br>" << endl;
  cout << "Notes:<br>" << endl;
  cout << "1.  Leaving both fields blank will list all entries in the log.<br>" << endl;
  cout << "2.  The object name field should use the % wildcard character.  Example: %shield%<br>" << endl;
  cout << "3.  The script ignores the other filters if the VNum field is used.<br>" << endl;
}

int main(int argc, char **argv)
{
  sstring my_query;
  unsigned int count = 0;

  toggleInfo.loadToggles();

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
    my_query = "SELECT l.vnum, substr(lower(i.name), 6) AS objtype, o.name, l.loadtime::TIMESTAMP(0), l.objcount FROM objlog l LEFT OUTER JOIN obj o USING (vnum) LEFT OUTER JOIN itemtypes i USING (\"type\")";
    if ((**vnum).empty() && (**name).empty()) {
      if ((**object_type) == "ALL") {
        my_query += " ORDER BY l.loadtime";
        db.query(my_query.c_str());
      } else {
        my_query += " WHERE lower(i.name) = lower('ITEM_' || '%s') ORDER BY l.loadtime";
        db.query(my_query.c_str(), (**object_type).c_str());
      }
    } else if ((**vnum).empty()) {
      if ((**object_type) == "ALL") {
        my_query += " WHERE lower(o.name) LIKE lower('%s') ORDER BY l.loadtime";
        db.query(my_query.c_str(), (**name).c_str());
      } else {
        my_query += " WHERE lower(o.name) LIKE lower('%s') AND lower(i.name) = lower('ITEM_' || '%s') ORDER BY l.loadtime";
        db.query(my_query.c_str(), (**name).c_str(), (**object_type).c_str());
      }
    } else {
      my_query += " WHERE l.vnum=%i ORDER BY l.loadtime";
      db.query(my_query.c_str(), convertTo<int>(**vnum));
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
