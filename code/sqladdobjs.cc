#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"

int main(int argc, char **argv)
{
  TDatabase db_immo("immortal");
  TDatabase db_beta("sneezybeta");
  sstring immortal;
  vector<int>vnums;
  int action_flag;
  
  if((argc-1) < 2){
    printf("Usage: %s <immortal> <obj list>\n", argv[0]);;
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
      printf("Adding %i ('%s')\n", vnums[t], db_immo.getColumn("short_desc"));
      
      // fix strung and prototype bits
      action_flag=convertTo<int>(db_immo.getColumn("action_flag"));
      if(action_flag & (1<<2)){
	action_flag=action_flag - (1<<2);
      }
      
      if(action_flag & (1<<4)){
	action_flag=action_flag - (1<<4);
      }


      db_beta.query("delete from obj where vnum=%i", vnums[t]);
      db_beta.query("insert into obj values(%s, '%s', '%s', '%s', '%s', %s, %i, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
		    db_immo.getColumn("vnum"), db_immo.getColumn("name"),
		    db_immo.getColumn("short_desc"), db_immo.getColumn("long_desc"), 
		    db_immo.getColumn("action_desc"), db_immo.getColumn("type"),
		    action_flag, db_immo.getColumn("wear_flag"),
		    db_immo.getColumn("val0"), db_immo.getColumn("val1"), 
		    db_immo.getColumn("val2"), db_immo.getColumn("val3"),
		    db_immo.getColumn("weight"), db_immo.getColumn("price"),
		    db_immo.getColumn("can_be_seen"), db_immo.getColumn("spec_proc"), 
		    db_immo.getColumn("max_exist"), db_immo.getColumn("max_struct"),
		    db_immo.getColumn("cur_struct"), db_immo.getColumn("decay"),
		    db_immo.getColumn("volume"), db_immo.getColumn("material"));


    } else {
      printf("Not found: %i\n", vnums[t]);
    }

    //// objaffect
    db_beta.query("delete from objaffect where vnum=%i", vnums[t]);

    db_immo.query("select vnum, type, mod1, mod2 from objaffect where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

    while(db_immo.fetchRow()){
      db_beta.query("insert into objaffect values(%s, %s, %s, %s)", db_immo.getColumn("vnum"), db_immo.getColumn("type"), db_immo.getColumn("mod1"), db_immo.getColumn("mod2"));
    }      

    
    //// obj extra
    db_beta.query("delete from objextra where vnum=%i", vnums[t]);

    db_immo.query("select vnum, name, description from objextra where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

    while(db_immo.fetchRow()){
      db_beta.query("insert into objextra values(%s, '%s', '%s')", db_immo.getColumn("vnum"), db_immo.getColumn("name"), db_immo.getColumn("description"));
    }

  }

}



