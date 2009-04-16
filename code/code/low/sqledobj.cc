#include "toggle.h"
#include "sstring.h"
#include "configuration.h"
#include "database.h"
#include "lowtools.h"
#include <unistd.h>

int main(int argc, char **argv)
{
  Config::doConfiguration();
  TDatabase db(DB_SNEEZYBETA);
  std::vector<int>vnums;
  std::map<sstring,sstring>val;
  char file[]="/tmp/sqledobjXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  toggleInfo.loadToggles();


  if(argc<=1){
    printf("Usage: sqledobj <vnum list>\n");
    printf("Example: sqledobj 13700-13780 13791 13798\n");
    exit(0);
  }

  if(!parse_num_args(argc-1, argv+1, vnums))
    exit(0);

  for(unsigned int i=0;i<vnums.size();++i){
    printf("Fetching data for object %i\n", vnums[i]);

    db.query("select * from obj where vnum=%i", vnums[i]);
    if(db.fetchRow()){
      buf = format("- obj\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("name: %s\n") % db["name"];  sbuf+=buf;
      buf = format("short_desc: %s\n") % db["short_desc"];  sbuf+=buf;
      buf = format("long_desc: %s\n") % db["long_desc"];  sbuf+=buf;
      buf = format("action_desc: %s\n") % db["action_desc"];  sbuf+=buf;
      buf = format("type: %s\n") % db["type"];  sbuf+=buf;
      buf = format("action_flag: %s\n") % db["action_flag"];  sbuf+=buf;
      buf = format("wear_flag: %s\n") % db["wear_flag"];  sbuf+=buf;
      buf = format("val0: %s\n") % db["val0"];  sbuf+=buf;
      buf = format("val1: %s\n") % db["val1"];  sbuf+=buf;
      buf = format("val2: %s\n") % db["val2"];  sbuf+=buf;
      buf = format("val3: %s\n") % db["val3"];  sbuf+=buf;
      buf = format("weight: %s\n") % db["weight"];  sbuf+=buf;
      buf = format("price: %s\n") % db["price"];  sbuf+=buf;
      buf = format("can_be_seen: %s\n") % db["can_be_seen"];  sbuf+=buf;
      buf = format("spec_proc: %s\n") % db["spec_proc"];  sbuf+=buf;
      buf = format("max_exist: %s\n") % db["max_exist"];  sbuf+=buf;
      buf = format("max_struct: %s\n") % db["max_struct"];  sbuf+=buf;
      buf = format("cur_struct: %s\n") % db["cur_struct"];  sbuf+=buf;
      buf = format("decay: %s\n") % db["decay"];  sbuf+=buf;
      buf = format("volume: %s\n") % db["volume"];  sbuf+=buf;
      buf = format("material: %s\n") % db["material"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from objextra where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- objextra\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("name: %s\n") % db["name"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from objaffect where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- objaffect\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("type: %s\n") % db["type"];  sbuf+=buf;
      buf = format("mod1: %s\n") % db["mod1"];  sbuf+=buf;
      buf = format("mod2: %s\n") % db["mod2"];  sbuf+=buf;
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


  printf("Insert object%s? [y/n] ", vnums.size()>1?"s":"");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Object%s not inserted.\n", vnums.size()>1?"s":"");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert object%s? [y/n] ", vnums.size()>1?"s":"");
    }
  }

  buf = format("/bin/cp -f %s %s.backup") % file % file;
  system(buf.c_str());


  int i=0;

  while(++i){
    val=parse_data_file(file, i);
    if(val["vnum"]=="EOM")
      break;
    
    if(val["DATATYPE"]=="obj"){
      printf("replacing object %s\n", val["vnum"].c_str());
      db.query("delete from obj where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from objextra where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from objaffect where vnum=%s",
	       val["vnum"].c_str());

      
      db.query("insert into obj (vnum,name,short_desc,long_desc,action_desc,type,action_flag,wear_flag,val0,val1,val2,val3,weight,price,can_be_seen,spec_proc,max_exist,max_struct,cur_struct,decay,volume,material) values (%s,'%s','%s','%s','%s',%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	       val["vnum"].c_str(),val["name"].c_str(),val["short_desc"].c_str(),val["long_desc"].c_str(),val["action_desc"].c_str(),val["type"].c_str(),val["action_flag"].c_str(),val["wear_flag"].c_str(),val["val0"].c_str(),val["val1"].c_str(),val["val2"].c_str(),val["val3"].c_str(),val["weight"].c_str(),val["price"].c_str(),val["can_be_seen"].c_str(),val["spec_proc"].c_str(),val["max_exist"].c_str(),val["max_struct"].c_str(),val["cur_struct"].c_str(),val["decay"].c_str(),val["volume"].c_str(),val["material"].c_str());
      
    } else if(val["DATATYPE"]=="objextra"){
      printf("replacing objextra %s\n", val["vnum"].c_str());
      
      db.query("insert into objextra (vnum, name, description) values (%s,'%s','%s')", val["vnum"].c_str(), val["name"].c_str(), val["description"].c_str());
      
    } else if(val["DATATYPE"]=="objaffect"){
      printf("replacing objaffect %s\n", val["vnum"].c_str());
      
      db.query("insert into objaffect (vnum,type,mod1,mod2) values (%s,%s,%s,%s)",
	       val["vnum"].c_str(), val["type"].c_str(),val["mod1"].c_str(),val["mod2"].c_str());

    }
    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
