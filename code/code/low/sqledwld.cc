#include "configuration.h"
#include "database.h"
#include "toggle.h"
#include "lowtools.h"
#include <unistd.h>
#include "sstring.h"
#include "parse.h"
#include "extern.h"

int txt2dir(sstring txt)
{
  for(int i=0;strcmp(dirs[i], "\n");++i){
    if(txt==dirs[i]){
      return i;
    }
  }
  return convertTo<int>(txt);
}

int main(int argc, char **argv)
{
  Config::doConfiguration();
  TDatabase db(DB_SNEEZYBETA);
  std::vector<int>vnums;
  std::map<sstring,sstring>val;
  char file[]="/tmp/sqledwldXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  toggleInfo.loadToggles();
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
      buf = format("- room\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("x:    %s\n") % db["x"];  sbuf+=buf;
      buf = format("y:    %s\n") % db["y"];  sbuf+=buf;
      buf = format("z:    %s\n") % db["z"];  sbuf+=buf;
      buf = format("name: %s\n") % db["name"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("room_flag: %s\n") % db["room_flag"];  sbuf+=buf;
      buf = format("sector:    %s\n") % db["sector"];  sbuf+=buf;
      buf = format("teletime:  %s\n") % db["teletime"];  sbuf+=buf;
      buf = format("teletarg:  %s\n") % db["teletarg"];  sbuf+=buf;
      buf = format("telelook:  %s\n") % db["telelook"];  sbuf+=buf;
      buf = format("river_speed: %s\n") % db["river_speed"];  sbuf+=buf;
      buf = format("river_dir: %s\n") % db["river_dir"];  sbuf+=buf;
      buf = format("capacity:  %s\n") % db["capacity"];  sbuf+=buf;
      buf = format("height:    %s\n") % db["height"];  sbuf+=buf;
      buf = format("spec:    %s\n") % db["spec"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from roomextra where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- roomextra\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("name: %s\n") % db["name"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from roomexit where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- roomexit\n"); sbuf+=buf;
      buf = format("vnum:      %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("direction: %s\n") % dirs[convertTo<int>(db["direction"])];
      sbuf+=buf;
      buf = format("name:      %s\n") % db["name"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("type:      %s\n") % db["type"];  sbuf+=buf;
      buf = format("condition_flag: %s\n") % db["condition_flag"]; sbuf+=buf;
      buf = format("lock_difficulty: %s\n") % db["lock_difficulty"]; sbuf+=buf;
      buf = format("weight:    %s\n") % db["weight"];  sbuf+=buf;
      buf = format("key_num:   %s\n") % db["key_num"];  sbuf+=buf;
      buf = format("destination:      %s\n") % db["destination"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }
  }


  mkstemp(file);
  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "%s", sbuf.c_str());
  fclose(tmpfile);

  printf("Opening editor.\n");
  buf = format("$EDITOR %s") % file;
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

  buf = format("/bin/cp -f %s %s.backup") % file % file;
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

  
      
      db.query("insert into room (vnum,x,y,z,name,description,room_flag,sector,teletime,teletarg,telelook,river_speed,river_dir,capacity,height,spec) values (%s,%s,%s,%s,'%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",
	       val["vnum"].c_str(),val["x"].c_str(),val["y"].c_str(),val["z"].c_str(),val["name"].c_str(),val["description"].c_str(),val["room_flag"].c_str(),val["sector"].c_str(),val["teletime"].c_str(),val["teletarg"].c_str(),val["telelook"].c_str(),val["river_speed"].c_str(),val["river_dir"].c_str(),val["capacity"].c_str(),val["height"].c_str(),val["spec"].c_str());
    } else if(val["DATATYPE"]=="roomextra"){
      printf("replacing roomextra %s\n", val["vnum"].c_str());
      
      db.query("insert into roomextra (vnum, name, description) values (%s,'%s','%s')", val["vnum"].c_str(), val["name"].c_str(), val["description"].c_str());
      
    } else if(val["DATATYPE"]=="roomexit"){
      printf("replacing roomexit %s\n", val["vnum"].c_str());
      
      db.query("insert into roomexit (vnum,direction,name,description,type,condition_flag,lock_difficulty,weight,key_num,destination) values (%s,%i,'%s','%s',%s,%s,%s,%s,%s,%s)", 
	       val["vnum"].c_str(),txt2dir(val["direction"]),val["name"].c_str(),val["description"].c_str(),val["type"].c_str(),val["condition_flag"].c_str(),val["lock_difficulty"].c_str(),val["weight"].c_str(),val["key_num"].c_str(),val["destination"].c_str());
    }
    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
