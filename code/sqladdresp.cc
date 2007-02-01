#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"

int main(int argc, char **argv)
{
  TDatabase db_immo(DB_IMMORTAL);
  TDatabase db_beta(DB_SNEEZYBETA);
  sstring immortal;
  vector<int>vnums;

  toggleInfo.loadToggles();
  
  if((argc-1) < 1){
    printf("Usage: %s <immortal> <resp list>\n", argv[0]);
    exit(0);
  }

  immortal=argv[1];
  
  if(!parse_num_args(argc-2, argv+2, vnums))
    exit(0);

  printf("Processing responses for %s\n", immortal.c_str());

  // loop through resp nums
  for(unsigned int t=0;t<vnums.size();t++){
    //// resp
    db_immo.query("select vnum, response from mobresponses where owner='%s' and vnum=%i",
		  immortal.c_str(), vnums[t]);
    
    if(db_immo.fetchRow()){
      printf("Adding %i ('%s')\n", vnums[t], db_immo["vnum"].c_str());

      db_beta.query("delete from mobresponses where vnum=%i", vnums[t]);
      db_beta.query("insert into mobresponses (vnum, response) values (%s,'%s')",
		    db_immo["vnum"].c_str(), db_immo["response"].c_str());
    } else {
      printf("Not found: %i\n", vnums[t]);
    }

  }

}



