#include "database.h"
#include "session.cgi.h"
#include "shop.h"

#include <vector>
#include <map>
#include <list>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPPlainHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>

#include <sys/types.h>

using namespace cgicc;

void sendJavaScript();

void sendShoplist(int);
void sendShowShop(int, int);
void sendShowLogs(int, int);
void sendShowLogsRaw(int, int);
void sendShowLogsArchive(int, int);
void sendShowLogsRawArchive(int, int);

int main(int argc, char **argv)
{
  // trick the DB code into use prod database
  gamePort=Config::Port::PROD;

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "shopinfo.cgi");
    return 0;
  } else {
    if(state_form == cgi.getElements().end() || **state_form == "main"){
      sendShoplist(session.getAccountID());
	    //      sendShoplist(664);
      return 0;
    } else if(**state_form == "showlogs"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
      sendShowLogs(session.getAccountID(), convertTo<int>(**shop_nr));
      return 0;
    } else if(**state_form == "showlogsraw"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
      sendShowLogsRaw(session.getAccountID(), convertTo<int>(**shop_nr));
      return 0;
    } else if(**state_form == "showlogsarchive"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
      sendShowLogsArchive(session.getAccountID(), convertTo<int>(**shop_nr));
      return 0;
    } else if(**state_form == "showlogsrawarchive"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
      sendShowLogsRawArchive(session.getAccountID(), convertTo<int>(**shop_nr));
      return 0;
    } else if(**state_form == "showshop"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
      sendShowShop(session.getAccountID(), convertTo<int>(**shop_nr));
      return 0;
    } else if(**state_form == "logout"){
      session.logout();
      cout << HTTPRedirectHeader("shopinfo.cgi").setCookie(session.getCookie());
      cout << endl;
      return 0;
    }

    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Shopinfo") << endl;
    cout << head() << body() << endl;
    cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
    cout << **state_form << endl;
    cout << body() << endl;
    cout << html() << endl;

    return 0;
  }  

  // shouldn't get here
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;
  cout << "This is bad.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;

}


void sendShowLogsRaw(int account_id, int shop_nr)
{
  TDatabase db(DB_SNEEZY);

  // check permissions first
  db.query("\
select 1 \
from \
  shop s, \
  corporation c, \
  shopowned so left outer join shopownedaccess soa on \
  (so.shop_nr=soa.shop_nr), \
  player p, corpaccess ca \
where \
  ca.corp_id=so.corp_id and \
  (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and \
  s.shop_nr=so.shop_nr and \
  c.corp_id=ca.corp_id and \
  so.shop_nr=%i and \
  p.account_id=%i",
	   shop_nr, account_id);


  if(!db.fetchRow()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Shopinfo") << endl;
    cout << head() << body() << endl;
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }
  

  cout << HTTPPlainHeader() << endl;
  
  
  db.query("select sl.shop_nr, sl.name, sl.action, sl.item, sl.talens, sl.shoptalens, sl.shopvalue, sl.logtime, sl.itemcount from shoplog sl, shopowned so where sl.shop_nr=so.shop_nr and sl.shop_nr=%i order by sl.logtime desc", shop_nr);
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


void sendShowLogs(int account_id, int shop_nr)
{
  TDatabase db(DB_SNEEZY);

  // check permissions first
  db.query("\
select 1 \
from \
  shop s, \
  corporation c, \
  shopowned so left outer join shopownedaccess soa on \
  (so.shop_nr=soa.shop_nr), \
  player p, corpaccess ca \
where \
  ca.corp_id=so.corp_id and \
  (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and \
  s.shop_nr=so.shop_nr and \
  c.corp_id=ca.corp_id and \
  so.shop_nr=%i and \
  p.account_id=%i",
	   shop_nr, account_id);
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;


  if(!db.fetchRow()){
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }
  

  
  db.query("select sl.shop_nr, sl.name, sl.action, sl.item, sl.talens, sl.shoptalens, sl.shopvalue, sl.logtime, sl.itemcount from shoplog sl, shopowned so where sl.shop_nr=so.shop_nr and sl.shop_nr=%i order by sl.logtime desc", shop_nr);

  cout << "<table border=1><tr>";

  cout << "<td>shop_nr</td><td>name</td><td>action</td><td>item</td><td>talens</td><td>shoptalens</td><td>shopvalue</td><td>logtime</td><td>itemcount</td></tr>" << endl;
  
  while(db.fetchRow()){
    cout << "<tr>";
    cout << "<td>" << stripColorCodes(db["shop_nr"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["name"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["action"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["item"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["talens"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["shoptalens"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["shopvalue"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["logtime"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["itemcount"]) << "</td></tr>" << endl;
  }
  cout << "</table>";

  cout << body() << endl;
  cout << html() << endl; 

}  


void sendShowLogsRawArchive(int account_id, int shop_nr)
{
  TDatabase db(DB_SNEEZY);

  // check permissions first
  db.query("\
select 1 \
from \
  shop s, \
  corporation c, \
  shopowned so left outer join shopownedaccess soa on \
  (so.shop_nr=soa.shop_nr), \
  player p, corpaccess ca \
where \
  ca.corp_id=so.corp_id and \
  (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and \
  s.shop_nr=so.shop_nr and \
  c.corp_id=ca.corp_id and \
  so.shop_nr=%i and \
  p.account_id=%i",
	   shop_nr, account_id);


  if(!db.fetchRow()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Shopinfo") << endl;
    cout << head() << body() << endl;
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }
  

  cout << HTTPPlainHeader() << endl;
  
  
  db.query("select sl.shop_nr, sl.name, sl.action, sl.item, sl.talens, sl.shoptalens, sl.shopvalue, sl.logtime, sl.itemcount from shoplogarchive sl, shopowned so where sl.shop_nr=so.shop_nr and sl.shop_nr=%i order by sl.logtime desc", shop_nr);
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


void sendShowLogsArchive(int account_id, int shop_nr)
{
  TDatabase db(DB_SNEEZY);

  // check permissions first
  db.query("\
select 1 \
from \
  shop s, \
  corporation c, \
  shopowned so left outer join shopownedaccess soa on \
  (so.shop_nr=soa.shop_nr), \
  player p, corpaccess ca \
where \
  ca.corp_id=so.corp_id and \
  (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and \
  s.shop_nr=so.shop_nr and \
  c.corp_id=ca.corp_id and \
  so.shop_nr=%i and \
  p.account_id=%i",
	   shop_nr, account_id);
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;


  if(!db.fetchRow()){
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }
  

  
  db.query("select sl.shop_nr, sl.name, sl.action, sl.item, sl.talens, sl.shoptalens, sl.shopvalue, sl.logtime, sl.itemcount from shoplogarchive sl, shopowned so where sl.shop_nr=so.shop_nr and sl.shop_nr=%i order by sl.logtime desc", shop_nr);

  cout << "<table border=1><tr>";

  cout << "<td>shop_nr</td><td>name</td><td>action</td><td>item</td><td>talens</td><td>shoptalens</td><td>shopvalue</td><td>logtime</td><td>itemcount</td></tr>" << endl;
  
  while(db.fetchRow()){
    cout << "<tr>";
    cout << "<td>" << stripColorCodes(db["shop_nr"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["name"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["action"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["item"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["talens"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["shoptalens"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["shopvalue"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["logtime"]) << "</td>";
    cout << "<td>" << stripColorCodes(db["itemcount"]) << "</td></tr>" << endl;
  }
  cout << "</table>";

  cout << body() << endl;
  cout << html() << endl; 

}  




void sendShowShop(int account_id, int shop_nr)
{
  TDatabase db(DB_SNEEZY);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;


  // check permissions first
  db.query("\
select 1 \
from \
  shop s, \
  corporation c, \
  shopowned so left outer join shopownedaccess soa on \
  (so.shop_nr=soa.shop_nr), \
  player p, corpaccess ca \
where \
  ca.corp_id=so.corp_id and \
  (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and \
  s.shop_nr=so.shop_nr and \
  c.corp_id=ca.corp_id and \
  so.shop_nr=%i and \
  p.account_id=%i",
	   shop_nr, account_id);

  if(!db.fetchRow()){
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }


  db.query("\
select \
  distinct so.shop_nr, so.no_such_item1, so.no_such_item2, so.do_not_buy, \
  so.missing_cash1, so.missing_cash2, so.message_buy, so.message_sell, \
  so.max_num, so.dividend, rtax.name as taxedby, s.gold, so.reserve_min, \
  so.reserve_max, so.profit_buy, so.profit_sell, r.name as shopname, \
  c.name as corpname, p.name as playername \
from \
  shop s left outer join shopownedtax sot on (sot.shop_nr=s.shop_nr) \
         left outer join shop stax on (stax.shop_nr=so.tax_nr) \
         left outer join room rtax on (rtax.vnum=stax.in_room), \
  corporation c, room r, \
  shopowned so, \
  player p \
where \
  p.account_id=%i and so.corp_id=c.corp_id and \
  r.vnum=s.in_room and s.shop_nr=so.shop_nr and so.shop_nr=%i \
order by r.name",
	   account_id, shop_nr);


  if(!db.fetchRow()){
    cout << "Shop not found or you don't have permission.";
    cout << body() << endl;
    cout << html() << endl; 
    return;
  }

  
  cout << "<table border=1>" << endl;
  cout << "<tr><td>Shop #" << db["shop_nr"] << "</td><td>" << db["shopname"];
  cout << "</td><td>" << db["corpname"] << "</td></tr>" << endl;

  cout << "<tr><td>Cash</td><td>" << talenDisplay(convertTo<int>(db["gold"]));
  cout << "</td>";
  cout << "<td>Taxed by: " << db["taxedby"] << "</td></tr>" << endl;

  cout << "<tr><td>Ratios</td><td>Buy: " << db["profit_buy"];
  cout << "</td><td>Sell: " << db["profit_sell"] << "</td></tr>" << endl;

  cout << "<tr><td>Reserve</td><td>Min: " << db["reserve_min"] << "</td>";
  cout << "<td>Max: " << db["reserve_max"] << "</td></tr>" << endl;

  cout << "<tr><td>Dividend</td><td>" << db["dividend"] << "</td></tr>" <<endl;

  cout << "<tr><td>Max Inv.</td><td>" << db["max_num"] << "</td></tr>" << endl;

  cout << "<tr><td colspan=3 align=center>Custom Strings</td></tr>" << endl;

  if(!db["no_such_item1"].empty()){
    cout << "<tr><td>no_such_item1</td><td colspan=2>" << db["no_such_item1"];
    cout << "</td></tr>" << endl;
  }
  if(!db["no_such_item2"].empty()){
    cout << "<tr><td>no_such_item2</td><td colspan=2>" << db["no_such_item2"];
    cout << "</td></tr>" << endl;
  }
  if(!db["do_not_buy"].empty()){
    cout << "<tr><td>do_not_buy</td><td colspan=2>" << db["do_not_buy"];
    cout << "</td></tr>" << endl;
  }
  if(!db["missing_cash1"].empty()){
    cout << "<tr><td>missing_cash1</td><td colspan=2>" << db["missing_cash1"];
    cout << "</td></tr>" << endl;
  }
  if(!db["missing_cash2"].empty()){
    cout << "<tr><td>missing_cash2</td><td colspan=2>" << db["missing_cash2"];
    cout << "</td></tr>" << endl;
  }
  if(!db["message_buy"].empty()){
    cout << "<tr><td>message_buy</td><td colspan=2>" << db["message_buy"];
    cout << "</td></tr>" << endl;
  }
  if(!db["message_sell"].empty()){
    cout << "<tr><td>message_sell</td><td colspan=2>" << db["message_sell"];
    cout << "</td></tr>" << endl;
  }
  
  cout << "</table>" << endl;


  /////// shop access
  db.query("select name, access from shopownedaccess where shop_nr=%i",
	   shop_nr);

  cout << "<hr><table border=1><tr><td>Shop Access</td>";
  cout << "<td>owner</td><td>info</td><td>rates</td><td>give</td><td>";
  cout << "sell</td><td>access</td><td>logs</td><td>dividend</td></tr>";

  while(db.fetchRow()){
    cout << "<tr><td>" << db["name"] << "</td>" << endl;

    int access=convertTo<int>(db["access"]);
    
    cout << "<td>" << ((access & SHOPACCESS_OWNER)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_INFO)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_RATES)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_GIVE)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_SELL)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_ACCESS)?"true":"false") << "</td>";
    cout << "<td>" << ((access & SHOPACCESS_LOGS)?"true":"false") << "</td>";
    cout << "<td>" << ((access&SHOPACCESS_DIVIDEND)?"true":"false") << "</td>";
    
  }
  cout << "</table>" << endl;


  /////////// shop ratios
  db.query("select o.short_desc, sor.profit_buy, sor.profit_sell, sor.max_num from obj o, shopownedratios sor where sor.shop_nr=%i and o.vnum=sor.obj_nr", shop_nr);

  cout << "<hr><table border=1><tr><td colspan=4>Shop Ratios</td></tr>";
  cout << "<tr><td>object</td><td>buy</td><td>sell</td><td>max</td></tr>";

  while(db.fetchRow()){
    cout << "<tr><td>" << escape_html(db["short_desc"]) << "</td>" << endl;
    cout << "<td>" << db["profit_buy"] << "</td>" << endl;
    cout << "<td>" << db["profit_sell"] << "</td>" << endl;
    cout << "<td>" << db["max_num"] << "</td>" << endl;
    cout << "</tr>";
  }
  cout << "</table>" << endl;
  

  cout << body() << endl;
  cout << html() << endl; 
}

void sendCorpShopList(int account_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("select c.name as corpname, p.name as pname from player p, corporation c, corpaccess ca where p.account_id=%i and c.corp_id=ca.corp_id and ca.player_id=p.id order by c.name", account_id);

  std::map<sstring, sstring>name_list;

  while(db.fetchRow()){
    name_list[db["corpname"]]=name_list[db["corpname"]]+db["pname"];
    name_list[db["corpname"]]=name_list[db["corpname"]]+" ";
  }

  db.query("select distinct so.shop_nr, r.name as shopname, c.name as corpname, c.corp_id from corporation c, room r, shop s, shopowned so, corpaccess ca, player p where p.account_id=%i and ca.player_id=p.id and so.corp_id=ca.corp_id and s.shop_nr=so.shop_nr and s.in_room=r.vnum and c.corp_id=ca.corp_id order by c.name, r.name", account_id);

  cout << "Shops you have access to via corporations you belong to:<br>";
  cout << "<table border=1>" << endl;
  cout << "<tr><td>Shop</td><td>Player(s)</td><td>Corporation</td><td>Logs</td><td>Export Logs</td><td>Archived Logs</td><td>Export Archived Logs</td></tr>";

  while(db.fetchRow()){
      cout << "<tr><td>" << endl;
      cout << format("<a href=javascript:pickshop('%s','showshop')>") %
	db["shop_nr"];
      cout << db["shopname"] << "</a></td><td>" << endl;
      cout << name_list[db["corpname"]] << "</td><td>";
      cout << "<a href=corp_info.cgi?corp_id=" << db["corp_id"] << ">";
      cout << db["corpname"] << "</a></td><td>";
      cout << format("<a href=javascript:pickshop('%s','showlogs')>") %
	db["shop_nr"];
      cout << "logs" << "</td><td>";

      cout << format("<a href=javascript:pickshop('%s','showlogsraw')>") % 
	db["shop_nr"];
      cout << "logs" << "</td><td>";

      cout << format("<a href=javascript:pickshop('%s','showlogsarchive')>") %
	db["shop_nr"];
      cout << "logs" << "</td><td>";

      cout << format("<a href=javascript:pickshop('%s','showlogsrawarchive')>") % 
	db["shop_nr"];
      cout << "logs" << "</td></tr>";
  }
  cout << "</table>";

}

void sendAccessShopList(int account_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("select p.name as pname, r.name as shopname, soa.access  from shopownedaccess soa, room r, shop s, shopowned so, player p where p.account_id=%i and s.shop_nr=so.shop_nr and s.in_room=r.vnum and lower(p.name)=lower(soa.name) and soa.shop_nr=s.shop_nr order by r.name", account_id);

  std::map<sstring, sstring>name_list;

  while(db.fetchRow()){
    name_list[db["shopname"]]+=format("%s (%s) ") % db["pname"] % db["access"];
  }

  db.query("select distinct so.shop_nr, r.name as shopname, c.name as corpname, c.corp_id from shopownedaccess soa, corporation c, room r, shop s, shopowned so, player p where p.account_id=%i and so.corp_id=c.corp_id and s.shop_nr=so.shop_nr and s.in_room=r.vnum and lower(p.name)=lower(soa.name) and soa.shop_nr=s.shop_nr order by c.name, r.name", account_id);

  cout << "<p>";
  cout << "Shops you have access to via individual shop permissions:<br>";
  cout << "<table border=1>" << endl;
  cout << "<tr><td>Shop</td><td>Player(s) (Access)</td><td>Corporation</td><td>Logs</td><td>Export logs</td></tr>";

  while(db.fetchRow()){
      cout << "<tr><td>" << endl;
      cout << format("<a href=javascript:pickshop('%s','showshop')>") %
	db["shop_nr"];
      cout << db["shopname"] << "</a></td><td>" << endl;
      cout << name_list[db["shopname"]] << "</td><td>";
      cout << "<a href=corp_info.cgi?corp_id=" << db["corp_id"] << ">";
      cout << db["corpname"] << "</a></td><td>";
      cout << format("<a href=javascript:pickshop('%s','showlogs')>") %
	db["shop_nr"];
      cout << "logs" << "</td><td>";
      cout << format("<a href=javascript:pickshop('%s','showlogsraw')>") % 
	db["shop_nr"];
      cout << "logs" << "</td></tr>";
  }

  cout << "</table>" << endl;
}

void sendShoplist(int account_id){
  TDatabase db(DB_SNEEZY);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=shopinfo.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"shopinfo.cgi\" method=post name=pickshop>" << endl;
  cout << "<input type=hidden name=shop_nr>" << endl;
  cout << "<input type=hidden name=state value=foo>" << endl;

  sendCorpShopList(account_id);

  sendAccessShopList(account_id);

  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}




void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  // this function is for making links emulate submits in shop selection
  cout << "function pickshop(shop_nr, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickshop.state.value = state;" << endl;
  cout << "document.pickshop.shop_nr.value = shop_nr;" << endl;
  cout << "document.pickshop.submit();" << endl;
  cout << "}" << endl;

  cout << "-->" << endl;
  cout << "</script>" << endl;


}
