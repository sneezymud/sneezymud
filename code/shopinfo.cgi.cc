#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"
#include "shop.h"

#include <map>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>

#include <sys/types.h>
#include <md5.h>

using namespace cgicc;

void sendJavaScript();

void sendLogin();
void sendLoginCheck(Cgicc cgi, TSession);

void sendShoplist(int);
void sendShowShop(int, int);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  toggleInfo.loadToggles();

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "shopinfo");

  if(!session.isValid()){
    if(state_form == cgi.getElements().end() ||
       **state_form != "logincheck"){
      sendLogin();
      return 0;
    } else {
      sendLoginCheck(cgi, session);
      return 0;
    }
  } else {
    if(state_form == cgi.getElements().end() || **state_form == "main"){
            sendShoplist(session.getAccountID());
      //sendShoplist(664);
      return 0;
    } else if(**state_form == "showshop"){
      form_iterator shop_nr=cgi.getElement("shop_nr");
            sendShowShop(session.getAccountID(), convertTo<int>(**shop_nr));
      //      sendShowShop(664, convertTo<int>(**shop_nr));
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
         left outer join shop stax on (stax.shop_nr=sot.tax_nr) \
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

void sendShoplist(int account_id){
  TDatabase db(DB_SNEEZY);

  db.query("select distinct so.shop_nr, r.name as shopname, c.name as corpname, p.name as playername from corporation c, shop s, room r, shopowned so left outer join shopownedaccess soa on (so.shop_nr=soa.shop_nr), player p, corpaccess ca where ca.corp_id=so.corp_id and p.account_id=%i and (lower(p.name)=lower(soa.name) or ca.player_id=p.id) and r.vnum=s.in_room and s.shop_nr=so.shop_nr and c.corp_id=ca.corp_id order by r.name", account_id);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=shopinfo.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>" << endl;


  if(!db.fetchRow()){
    cout << "You don't have access to any shops." << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  cout << "<form action=\"shopinfo.cgi\" method=post name=pickshop>" << endl;
  cout << "<input type=hidden name=shop_nr>" << endl;
  cout << "<input type=hidden name=state value=showshop>" << endl;
  cout << "<table border=1>" << endl;
  cout << "<tr><td>Shop</td><td>Player</td><td>Corporation</td></tr>";

  do {
      cout << "<tr><td>" << endl;
      cout << "<a href=javascript:pickshop('" << db["shop_nr"] << "')>";
      cout << db["shopname"] << "</a></td><td>" << endl;
      cout << db["playername"] << "</td><td>";
      cout << db["corpname"] << "</td></tr>";
  } while(db.fetchRow());

  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}


void sendLoginCheck(Cgicc cgi, TSession session)
{
  // validate, create session cookie + db entry, redirect to main
  sstring name=**(cgi.getElement("account"));
  sstring passwd=**(cgi.getElement("passwd"));
  form_iterator autologin=cgi.getElement("autologin");

  if(!session.checkPasswd(name, passwd)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Shopinfo") << endl;
    cout << head() << body() << endl;
    cout << "Password incorrect or account not found.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  if(autologin == cgi.getElements().end()){
    session.createSession();
  } else {
    // log them in for a year or so
    session.createSession(60*60*24*365);
  }

  
  cout << HTTPRedirectHeader("shopinfo.cgi").setCookie(session.getCookie());
  cout << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;
  cout << "Good job, you logged in.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;
}


void sendLogin()
{
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Shopinfo") << endl;
  cout << head() << body() << endl;

  cout << "<form action=\"shopinfo.cgi\" method=post>" << endl;
  cout << "<table>" << endl;
  cout << "<tr><td>Account</td>" << endl;
  cout << "<td><input type=text name=account></td></tr>" << endl;
  cout << "<tr><td>Password</td>" << endl;
  cout << "<td><input type=password name=passwd></td></tr>" << endl;
  cout << "<tr><td><input type=checkbox name=autologin></td>" << endl;
  cout << "<td>Log me on automatically each visit.</td></tr>" << endl;
  cout << "<tr><td colspan=2><input type=submit value=Login></td></tr>" <<endl;
  cout << "</table>" << endl;
  cout << "<input type=hidden name=state value=logincheck>" << endl;
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}


void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  // this function is for making links emulate submits in shop selection
  cout << "function pickshop(shop_nr)" << endl;
  cout << "{" << endl;
  cout << "document.pickshop.shop_nr.value = shop_nr;" << endl;
  cout << "document.pickshop.submit();" << endl;
  cout << "}" << endl;
  cout << "-->" << endl;
  cout << "</script>" << endl;
}
