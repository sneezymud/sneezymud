#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"
#include <unistd.h>

int main(int argc, char **argv)
{
  TDatabase db(DB_SNEEZYBETA);
  vector<int>vnums;
  map<sstring,sstring>val;
  char file[]="/tmp/sqledwldXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  if(argc<=1){
    printf("Usage: sqledwld <vnum list>\n");
    printf("Example: sqledwld 13700-13780 13791 13798\n");
    exit(0);
  }

  if(!parse_num_args(argc-1, argv+1, vnums))
    exit(0);

  for(unsigned int i=0;i<vnums.size();++i){
    printf("Fetching data for room %i\n", vnums[i]);

    db.query("select * from room where vnum=%i", vnums[i]);
    if(db.fetchRow()){
      ssprintf(buf, "- room\n"); sbuf+=buf;
      ssprintf(buf, "vnum: %s\n", db["vnum"]);  sbuf+=buf;
      ssprintf(buf, "x:    %s\n", db["x"]);  sbuf+=buf;
      ssprintf(buf, "y:    %s\n", db["y"]);  sbuf+=buf;
      ssprintf(buf, "z:    %s\n", db["z"]);  sbuf+=buf;
      ssprintf(buf, "name: %s\n", db["name"]);  sbuf+=buf;
      ssprintf(buf, "description~:\n%s~\n", db["description"]);  sbuf+=buf;
      ssprintf(buf, "room_flag: %s\n", db["room_flag"]);  sbuf+=buf;
      ssprintf(buf, "sector:    %s\n", db["sector"]);  sbuf+=buf;
      ssprintf(buf, "teletime:  %s\n", db["teletime"]);  sbuf+=buf;
      ssprintf(buf, "teletarg:  %s\n", db["teletarg"]);  sbuf+=buf;
      ssprintf(buf, "telelook:  %s\n", db["telelook"]);  sbuf+=buf;
      ssprintf(buf, "river_speed: %s\n", db["river_speed"]);  sbuf+=buf;
      ssprintf(buf, "river_dir: %s\n", db["river_dir"]);  sbuf+=buf;
      ssprintf(buf, "capacity:  %s\n", db["capacity"]);  sbuf+=buf;
      ssprintf(buf, "height:    %s\n", db["height"]);  sbuf+=buf;
      ssprintf(buf, "\n"); sbuf+=buf;
    }


    db.query("select * from roomextra where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      ssprintf(buf, "- roomextra\n"); sbuf+=buf;
      ssprintf(buf, "vnum: %s\n", db["vnum"]);  sbuf+=buf;
      ssprintf(buf, "name: %s\n", db["name"]);  sbuf+=buf;
      ssprintf(buf, "description~:\n%s~\n", db["description"]);  sbuf+=buf;
      ssprintf(buf, "\n"); sbuf+=buf;
    }


    db.query("select * from roomexit where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      ssprintf(buf, "- roomexit\n"); sbuf+=buf;
      ssprintf(buf, "vnum:      %s\n", db["vnum"]);  sbuf+=buf;
      ssprintf(buf, "direction: %s\n", db["direction"]);  sbuf+=buf;
      ssprintf(buf, "name:      %s\n", db["name"]);  sbuf+=buf;
      ssprintf(buf, "description~:\n%s~\n", db["description"]);  sbuf+=buf;
      ssprintf(buf, "type:      %s\n", db["type"]);  sbuf+=buf;
      ssprintf(buf, "condition_flag: %s\n", db["condition_flag"]); sbuf+=buf;
      ssprintf(buf, "lock_difficulty: %s\n", db["lock_difficulty"]); sbuf+=buf;
      ssprintf(buf, "weight:    %s\n", db["weight"]);  sbuf+=buf;
      ssprintf(buf, "key_num:   %s\n", db["key_num"]);  sbuf+=buf;
      ssprintf(buf, "destination:      %s\n", db["destination"]);  sbuf+=buf;
      ssprintf(buf, "\n"); sbuf+=buf;
    }
  }


  mkstemp(file);
  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "%s", sbuf.c_str());
  fclose(tmpfile);

  printf("Opening editor.\n");
  ssprintf(buf, "$EDITOR %s", file);
  system(buf.c_str());


  printf("Insert room%s? [y/n] ", vnums.size()>1?"s":"");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Room%s not inserted.\n", vnums.size()>1?"s":"");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert room%s? [y/n] ", vnums.size()>1?"s":"");
    }
  }

  ssprintf(buf, "/bin/cp -f %s %s.backup", file, file);
  system(buf.c_str());


  int i=0;

  while(++i){
    val=parse_data_file(file, i);
    if(val["vnum"]=="EOM")
      break;
    
    if(val["DATATYPE"]=="room"){
      printf("replacing room %s\n", val["vnum"].c_str());
      db.query("delete from room where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from roomextra where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from roomexit where vnum=%s",
	       val["vnum"].c_str());

  
      
      db.query("insert into room (vnum,x,y,z,name,description,room_flag,sector,teletime,teletarg,telelook,river_speed,river_dir,capacity,height,zone) values (%s,%s,%s,%s,'%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,-1)",
	       val["vnum"].c_str(),val["x"].c_str(),val["y"].c_str(),val["z"].c_str(),val["name"].c_str(),val["description"].c_str(),val["room_flag"].c_str(),val["sector"].c_str(),val["teletime"].c_str(),val["teletarg"].c_str(),val["telelook"].c_str(),val["river_speed"].c_str(),val["river_dir"].c_str(),val["capacity"].c_str(),val["height"].c_str());
    } else if(val["DATATYPE"]=="roomextra"){
      printf("replacing roomextra %s\n", val["vnum"].c_str());
      
      db.query("insert into roomextra (vnum, name, description) values (%s,'%s','%s')", val["vnum"].c_str(), val["name"].c_str(), val["description"].c_str());
      
    } else if(val["DATATYPE"]=="roomexit"){
      printf("replacing roomexit %s\n", val["vnum"].c_str());
      
      db.query("insert into roomexit (vnum,direction,name,description,type,condition_flag,lock_difficulty,weight,key_num,destination) values (%s,%s,'%s','%s',%s,%s,%s,%s,%s,%s)", 
	       val["vnum"].c_str(),val["direction"].c_str(),val["name"].c_str(),val["description"].c_str(),val["type"].c_str(),val["condition_flag"].c_str(),val["lock_difficulty"].c_str(),val["weight"].c_str(),val["key_num"].c_str(),val["destination"].c_str());
    }
    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
