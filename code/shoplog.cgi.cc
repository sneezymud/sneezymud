#include "stdsneezy.h"
#include "database.h"
#include "shop.h"

#include <vector>
#include <string>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;


int main(int argc, char **argv){
  Cgicc cgi;
  TDatabase db(DB_SNEEZYPROD);
  sstring buf;

  form_iterator name = cgi.getElement("name");
  form_iterator pw = cgi.getElement("pw");
  form_iterator shop_nr = cgi.getElement("shop_nr");

  if(name == cgi.getElements().end() ||
     pw == cgi.getElements().end()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("shop logs")) << endl;
    cout << body() << endl;

    cout << "<form method=post action=\"/shoplog.cgi\">" << endl;
    cout << "Name: <input type=text name=name>" << endl;
    cout << "Password: <input type=password name=pw><br>" << endl;
    cout << "<input type=submit>" << endl;
    cout << body() << html();
  } else if(shop_nr == cgi.getElements().end()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("shop logs")) << endl;
    cout << body() << endl;

    cout << "<form method=post action=\"/shoplog.cgi\">" << endl;
    cout << "Select a shop: <select name=shop_nr>" << endl;

    db.query("select soa.shop_nr as shop_nr from shopownedaccess soa, shopowned so where lower(name) = lower('%s') and soa.shop_nr=so.shop_nr and so.password='%s' and so.password is not null and so.password != ''", (**name).c_str(), (**pw).c_str());
    
    while(db.fetchRow()){
      cout << "<option value=" << db["shop_nr"];
      cout << "> " << db["shop_nr"] << endl;
    }

    db.query("select so.shop_nr as shop_nr from corpaccess ca, shopowned so where lower(name)=lower('%s') and so.corp_id=ca.corp_id and so.password='%s' and so.password is not null and so.password != ''", (**name).c_str(), (**pw).c_str());

    while(db.fetchRow()){
      cout << "<option value=" << db["shop_nr"];
      cout << "> " << db["shop_nr"] << endl;
    }


    cout << "</select>";
    cout << "<input type=hidden name=name value=" << **name << ">";
    cout << "<input type=hidden name=pw value=" << **pw << ">";
    cout << "<input type=submit>" << endl;
    cout << body() << html();
  } else {
    db.query("select sl.shop_nr, sl.name, sl.action, sl.item, sl.talens, sl.shoptalens, sl.shopvalue, sl.logtime, sl.itemcount from shoplog sl, shopowned so where sl.shop_nr=so.shop_nr and so.password='%s' and so.password is not null and so.password != '' and sl.shop_nr=%i order by sl.logtime", (**pw).c_str(), convertTo<int>(**shop_nr));
    cout << "Content-type: text/plain\n\n";
    cout << "shop_nr, name, action, item, talens, shoptalens, shopvalue, logtime, itemcount\n";

    while(db.fetchRow()){
      cout << stripColorCodes(db["shop_nr"]) << ", ";
      cout << stripColorCodes(db["name"]) << ", ";
      cout << stripColorCodes(db["action"]) << ", ";
      cout << stripColorCodes(db["item"]) << ", ";
      cout << stripColorCodes(db["talens"]) << ", ";
      cout << stripColorCodes(db["shoptalens"]) << ", ";
      cout << stripColorCodes(db["shopvalue"]) << ", ";
      cout << stripColorCodes(db["logtime"]) << ", ";
      cout << stripColorCodes(db["itemcount"]) << endl;
    }
  }
  
}
