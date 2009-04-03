#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"

int main(int argc, char **argv)
{
  TDatabase db_immo(DB_IMMORTAL);
  TDatabase db_beta(DB_SNEEZYBETA);
  sstring immortal, block;
  vector<int>vnums;

  toggleInfo.loadToggles();
  
  if((argc-1) < 2){
    printf("Usage: %s <immortal> <block> <room list>\n", argv[0]);
    exit(0);
  }

  immortal=argv[1];
  block=argv[2];
  
  if(!parse_num_args(argc-3, argv+3, vnums))
    exit(0);

  printf("Processing rooms for %s\n", immortal.c_str());

  // loop through room nums
  for(unsigned int t=0;t<vnums.size();t++){
    //// room
    db_immo.query("select vnum, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec from room where owner='%s' and vnum=%i and block=%s",
		  immortal.c_str(), vnums[t], block.c_str());
    
    if(db_immo.fetchRow()){
      printf("Adding %i ('%s')\n", vnums[t], db_immo["name"].c_str());

      db_beta.query("delete from room where vnum=%i", vnums[t]);
      db_beta.query("insert into room (vnum,x,y,z,name,description,room_flag,sector,teletime,teletarg,telelook,river_speed,river_dir,capacity,height,spec) values (%s,%s,%s,%s, '%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",
		    db_immo["vnum"].c_str(), db_immo["x"].c_str(),
		    db_immo["y"].c_str(), db_immo["z"].c_str(),
		    db_immo["name"].c_str(), db_immo["description"].c_str(),
		    db_immo["room_flag"].c_str(), db_immo["sector"].c_str(),
		    db_immo["teletime"].c_str(), db_immo["teletarg"].c_str(),
		    db_immo["telelook"].c_str(),db_immo["river_speed"].c_str(),
		    db_immo["river_dir"].c_str(), db_immo["capacity"].c_str(),
		    db_immo["height"].c_str(), db_immo["spec"].c_str());


      //// roomextra
      db_beta.query("delete from roomextra where vnum=%i", vnums[t]);

      db_immo.query("select vnum, name, description from roomextra where owner='%s' and vnum=%i and block=%s", immortal.c_str(), vnums[t], block.c_str());

      while(db_immo.fetchRow()){
	db_beta.query("insert into roomextra (vnum, name, description) values (%s, '%s', '%s')", db_immo["vnum"].c_str(), db_immo["name"].c_str(), db_immo["description"].c_str());
      }      

    
      //// roomexit
      db_beta.query("delete from roomexit where vnum=%i", vnums[t]);

      db_immo.query("select vnum, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination from roomexit where owner='%s' and vnum=%i and block=%s", immortal.c_str(), vnums[t], block.c_str());

      while(db_immo.fetchRow()){
	db_beta.query("insert into roomexit (vnum,direction,name,description,type,condition_flag,lock_difficulty,weight,key_num,destination) values (%s, %s,'%s','%s',%s,%s,%s,%s,%s,%s)", 
		 db_immo["vnum"].c_str(), db_immo["direction"].c_str(), 
		 db_immo["name"].c_str(), db_immo["description"].c_str(), 
		 db_immo["type"].c_str(), db_immo["condition_flag"].c_str(), 
		 db_immo["lock_difficulty"].c_str(), db_immo["weight"].c_str(), 
		 db_immo["key_num"].c_str(), db_immo["destination"].c_str());

      }

    } else {
      printf("Not found: %i\n", vnums[t]);
    }

  }

}



