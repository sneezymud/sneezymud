#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"

int main(int argc, char **argv)
{
  TDatabase db_immo("immortal");
  TDatabase db_beta("sneezybeta");
  string immortal;
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
      printf("Adding %i\n", vnums[t]);
      
      // fix strung and prototype bits
      action_flag=atoi_safe(db_immo.getColumn(6));
      if(action_flag & (1<<2)){
	action_flag=action_flag - (1<<2);
      }
      
      if(action_flag & (1<<4)){
	action_flag=action_flag - (1<<4);
      }


      db_beta.query("delete from obj where vnum=%i", vnums[t]);
      db_beta.query("insert into obj values(%s, '%s', '%s', '%s', '%s', %s, %i, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)", db_immo.getColumn(0), db_immo.getColumn(1), db_immo.getColumn(2), db_immo.getColumn(3), db_immo.getColumn(4), db_immo.getColumn(5), action_flag, db_immo.getColumn(7), db_immo.getColumn(8), db_immo.getColumn(9), db_immo.getColumn(10), db_immo.getColumn(11), db_immo.getColumn(12), db_immo.getColumn(13), db_immo.getColumn(14), db_immo.getColumn(15), db_immo.getColumn(16), db_immo.getColumn(17), db_immo.getColumn(18), db_immo.getColumn(19), db_immo.getColumn(20), db_immo.getColumn(21));
    } else {
      printf("Not found: %i\n", vnums[t]);
    }

    //// objaffect
    db_beta.query("delete from objaffect where vnum=%i", vnums[t]);

    db_immo.query("select vnum, type, mod1, mod2 from objaffect where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

    while(db_immo.fetchRow()){
      db_beta.query("insert into objaffect values(%s, %s, %s, %s)", db_immo.getColumn(0), db_immo.getColumn(1), db_immo.getColumn(2), db_immo.getColumn(3));
    }      

    
    //// obj extra
    db_beta.query("delete from objextra where vnum=%i", vnums[t]);

    db_immo.query("select vnum, name, description from objextra where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

    while(db_immo.fetchRow()){
      db_beta.query("insert into objextra values(%s, '%s', '%s')", db_immo.getColumn(0), db_immo.getColumn(1), db_immo.getColumn(2));
    }

  }

}



