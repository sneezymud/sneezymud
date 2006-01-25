#include "stdsneezy.h"
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
  gamePort = PROD_GAMEPORT;
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

}

void sendCorporationsList()
{
  vector <corp_list_data> corp_list;
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
    cout << (fmt("<tr><td>%i</td><td><a href=\"corp_info.cgi?corp_id=%i\">%s</a></td><td>%s</td></tr>") %
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

