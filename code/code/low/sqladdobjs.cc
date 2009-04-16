#include "toggle.h"
#include "sstring.h"
#include "configuration.h"
#include "database.h"
#include "lowtools.h"
#include "parse.h"

int main(int argc, char **argv)
{
  Config::doConfiguration();
  TDatabase db_immo(DB_IMMORTAL);
  TDatabase db_beta(DB_SNEEZYBETA);
  sstring immortal;
  std::vector<int>vnums;
  int action_flag;

  toggleInfo.loadToggles();
  
  if((argc-1) < 2){
    printf("Usage: %s <immortal> <obj list>\n", argv[0]);
    exit(0);
  }

  immortal=argv[1];
  
  if(!parse_num_args(argc-2, argv+2, vnums))
    exit(0);

  printf("Processing items for %s\n", immortal.c_str());

  // loop through item nums
  for(unsigned int t=0;t<vnums.size();t++){
    //// obj
    db_immo.query("select vnum,name,short_desc,long_desc,action_desc,type,action_flag,wear_flag,val0,val1,val2,val3,weight,price,can_be_seen,spec_proc,max_exist,max_struct,cur_struct,decay,volume,material from obj where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);
    
    if(db_immo.fetchRow()){
      printf("Adding %i ('%s')\n", vnums[t], db_immo["short_desc"].c_str());
      
      // fix strung and prototype bits
      action_flag=convertTo<int>(db_immo["action_flag"]);
      if(action_flag & (1<<2)){
	action_flag=action_flag - (1<<2);
      }
      
      if(action_flag & (1<<4)){
	action_flag=action_flag - (1<<4);
      }


      db_beta.query("delete from obj where vnum=%i", vnums[t]);
      db_beta.query("insert into obj values(%s, '%s', '%s', '%s', '%s', %s, %i, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
		    db_immo["vnum"].c_str(), db_immo["name"].c_str(),
		    db_immo["short_desc"].c_str(), db_immo["long_desc"].c_str(), 
		    db_immo["action_desc"].c_str(), db_immo["type"].c_str(),
		    action_flag, db_immo["wear_flag"].c_str(),
		    db_immo["val0"].c_str(), db_immo["val1"].c_str(), 
		    db_immo["val2"].c_str(), db_immo["val3"].c_str(),
		    db_immo["weight"].c_str(), db_immo["price"].c_str(),
		    db_immo["can_be_seen"].c_str(), db_immo["spec_proc"].c_str(), 
		    db_immo["max_exist"].c_str(), db_immo["max_struct"].c_str(),
		    db_immo["cur_struct"].c_str(), db_immo["decay"].c_str(),
		    db_immo["volume"].c_str(), db_immo["material"].c_str());


      //// objaffect
      db_beta.query("delete from objaffect where vnum=%i", vnums[t]);

      db_immo.query("select vnum, type, mod1, mod2 from objaffect where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

      while(db_immo.fetchRow()){
	db_beta.query("insert into objaffect values(%s, %s, %s, %s)", db_immo["vnum"].c_str(), db_immo["type"].c_str(), db_immo["mod1"].c_str(), db_immo["mod2"].c_str());
      }      

    
      //// obj extra
      db_beta.query("delete from objextra where vnum=%i", vnums[t]);

      db_immo.query("select vnum, name, description from objextra where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

      while(db_immo.fetchRow()){
	db_beta.query("insert into objextra values(%s, '%s', '%s')", db_immo["vnum"].c_str(), db_immo["name"].c_str(), db_immo["description"].c_str());
      }

    } else {
      printf("Not found: %i\n", vnums[t]);
    }

  }

}



