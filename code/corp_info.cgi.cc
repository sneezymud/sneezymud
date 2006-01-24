#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"

#include <map>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;

TDatabase db(DB_SNEEZYPROD);
// TDatabase db(DB_SNEEZYBETA);
Cgicc cgi;


int main(int argc, char **argv)
{
  sstring my_query;
  multimap <int, sstring, std::greater<int> > m;

  toggleInfo.loadToggles();
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head(title("Corporations")) << endl;
  cout << body() << endl;

  m=getCorpListingData();

  cout << body() << endl;
  cout << html() << endl;
}

