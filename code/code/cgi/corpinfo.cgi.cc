#include "database.h"
#include "corporation.h"

#include <map>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;

Cgicc cgi;

void sendCorporationsList();
void sendCorporationInfo(int);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = Config::Port::PROD;
  toggleInfo.loadToggles();
  form_iterator corp_id_form=cgi.getElement("corp_id");

  cout << HTTPHTMLHeader() << endl;

  if(corp_id_form == cgi.getElements().end()){
    sendCorporationsList();
  } else {
    sendCorporationInfo(convertTo<int>(**corp_id_form));
  }
}

void sendCorporationInfo(int corp_id)
{
  TDatabase db(DB_SNEEZY);
  int value=0, gold=0, shopcount=0, banktalens=0, bank=4, bankowner=0;
  sstring buf, bankname;

  cout << html() << head(title("Corporation")) << endl;
  cout << body() << endl;

  if(!corp_id){
    cout << "No information for that corporation." << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }
  

  db.query("select c.name, sum(s.gold) as gold, b.talens as banktalens, count(s.shop_nr) as shops, bank, r.name as bankname, sob.corp_id as bankowner from shop sbank, room r, shopowned sob, corporation c left outer join shopownedcorpbank b on (c.corp_id=b.corp_id), shopowned so, shop s where sob.shop_nr=c.bank and c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr and r.vnum=sbank.in_room and sbank.shop_nr=c.bank group by c.corp_id, c.name, b.talens, c.bank, sob.corp_id, r.name order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    cout << "No information for that corporation." << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  cout << "<table border=1>";

  cout << (format("<tr><td>%-3i</td><td>%s</td></tr>") %
	   corp_id % db["name"]);

  bank=convertTo<int>(db["bank"]);
  bankname=db["bankname"];
  bankowner=convertTo<int>(db["bankowner"]);
  banktalens=convertTo<int>(db["banktalens"]);
  gold=convertTo<int>(db["gold"]);
  TCorporation corp(corp_id);
  value=corp.getAssets();
  shopcount=convertTo<int>(db["shops"]);

  // we own the bank, so don't count our money twice
  if(bankowner == corp_id)
    gold -= banktalens;
  
  cout << (format("<tr><td>Bank Talens</td><td>%12s</td></tr>") %
	     (format("%i") % banktalens).comify());
  cout << (format("<tr><td>Talens</td><td>%12s</td></tr>") % 
	     (format("%i") % gold).comify());
  cout << (format("<tr><td>Assets</td><td>%12s</td></tr>") % 
	     (format("%i") % value).comify());
  cout << (format("<tr><td>Shops (x1M)</td><td>%12s</td></tr>") % 
	     (format("%i") % (shopcount*1000000)).comify());
  cout << (format("<tr><td>Total value</td><td>%12s</td></tr>") %
	     (format("%i") % (banktalens+gold+value+(shopcount * 1000000))).comify());


  // officers
  db.query("select name from corpaccess where corp_id=%i", corp_id);
  
  buf="";
  while(db.fetchRow()){
    buf+=" ";
    buf+=db["name"];
  }
  cout << (format("<tr><td>Corporate officers are</td><td>%s</td></tr>") % buf);

  // bank
  //  if((tr=real_roomp(shop_index[bank].in_room))){
    cout << ("<tr><td>Corporate bank is</td><td>");
    cout << (format("%-3i| %s</td></tr>") % bank % bankname); //tr->getName());
    //  }

  // shops    
  db.query("select s.shop_nr, r.name, s.gold from shop s, shopowned so, room r where s.shop_nr=so.shop_nr and so.corp_id=%i and r.vnum=s.in_room order by s.gold desc", corp_id);
  
  cout << ("<tr><td>The following shops are owned by this corporation:</td><td></td></tr>");
  
  while(db.fetchRow()){
    //    if((tr=real_roomp(convertTo<int>(db["in_room"])))){
      gold=convertTo<int>(db["gold"]);
      cout << (format("<tr><td>%-3s</td><td>%s with %s talens.</td></tr>") %
		 db["shop_nr"] % db["name"] % talenDisplay(gold));
      //    }
  }
  
  cout << "</table>";
  cout << body() << endl;
  cout << html() << endl;

}

void sendCorporationsList()
{
  std::vector <corp_list_data> corp_list;
  multimap <int, int, std::greater<int> > m;
  multimap <int, int, std::greater<int> >::iterator it;

  cout << html() << head(title("Corporations")) << endl;
  cout << body() << endl;

  corp_list=getCorpListingData();
  
  for(unsigned int i=0;i<corp_list.size();++i){
    m.insert(pair<int,int>(corp_list[i].gold, i));
  }

  cout << "<table border=1>" << endl;
  cout << "<tr><td>ID</td><td>Name</td><td>Gold</td></tr>" << endl;
  for(it=m.begin();it!=m.end();++it){
    cout << (format("<tr><td>%i</td><td><a href=\"corpinfo.cgi?corp_id=%i\">%s</a></td><td>%s</td></tr>") %
	     corp_list[(int)((*it).second)].corp_id %
	     corp_list[(int)((*it).second)].corp_id %
	     corp_list[(int)((*it).second)].name %
	     talenDisplay((*it).first)).c_str();
    cout << endl;
  }
  cout << "</table>";

  cout << body() << endl;
  cout << html() << endl;
}

