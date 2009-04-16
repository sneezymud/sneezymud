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
  char file[]="/tmp/sqledmobXXXXXX";
  FILE *tmpfile;
  sstring buf, sbuf;
  int answer;

  toggleInfo.loadToggles();


  if(argc<=1){
    printf("Usage: sqledmob <vnum list>\n");
    printf("Example: sqledmob 13700-13780 13791 13798\n");
    exit(0);
  }

  if(!parse_num_args(argc-1, argv+1, vnums))
    exit(0);

  for(unsigned int i=0;i<vnums.size();++i){
    printf("Fetching data for mob %i\n", vnums[i]);

    db.query("select * from mob where vnum=%i", vnums[i]);
    if(db.fetchRow()){
      buf = format("- mob\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("name: %s\n") % db["name"];  sbuf+=buf;
      buf = format("short_desc: %s\n") % db["short_desc"];  sbuf+=buf;
      buf = format("long_desc~:\n%s~\n") % db["long_desc"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("actions: %s\n") % db["actions"];  sbuf+=buf;
      buf = format("affects: %s\n") % db["affects"];  sbuf+=buf;
      buf = format("faction: %s\n") % db["faction"];  sbuf+=buf;
      buf = format("fact_perc: %s\n") % db["fact_perc"];  sbuf+=buf;
      buf = format("letter: %s\n") % db["letter"];  sbuf+=buf;
      buf = format("attacks: %s\n") % db["attacks"];  sbuf+=buf;
      buf = format("class: %s\n") % db["class"];  sbuf+=buf;
      buf = format("level: %s\n") % db["level"];  sbuf+=buf;
      buf = format("tohit: %s\n") % db["tohit"];  sbuf+=buf;
      buf = format("ac: %s\n") % db["ac"];  sbuf+=buf;
      buf = format("hpbonus: %s\n") % db["hpbonus"];  sbuf+=buf;
      buf = format("damage_level: %s\n") % db["damage_level"];  sbuf+=buf;
      buf = format("damage_precision: %s\n") % db["damage_precision"];  sbuf+=buf;
      buf = format("gold: %s\n") % db["gold"];  sbuf+=buf;
      buf = format("race: %s\n") % db["race"];  sbuf+=buf;
      buf = format("weight: %s\n") % db["weight"];  sbuf+=buf;
      buf = format("height: %s\n") % db["height"];  sbuf+=buf;
      buf = format("str: %s\n") % db["str"];  sbuf+=buf;
      buf = format("bra: %s\n") % db["bra"];  sbuf+=buf;
      buf = format("con: %s\n") % db["con"];  sbuf+=buf;
      buf = format("dex: %s\n") % db["dex"];  sbuf+=buf;
      buf = format("agi: %s\n") % db["agi"];  sbuf+=buf;
      buf = format("intel: %s\n") % db["intel"];  sbuf+=buf;
      buf = format("wis: %s\n") % db["wis"];  sbuf+=buf;
      buf = format("foc: %s\n") % db["foc"];  sbuf+=buf;
      buf = format("per: %s\n") % db["per"];  sbuf+=buf;
      buf = format("cha: %s\n") % db["cha"];  sbuf+=buf;
      buf = format("kar: %s\n") % db["kar"];  sbuf+=buf;
      buf = format("spe: %s\n") % db["spe"];  sbuf+=buf;
      buf = format("pos: %s\n") % db["pos"];  sbuf+=buf;
      buf = format("def_position: %s\n") % db["def_position"];  sbuf+=buf;
      buf = format("sex: %s\n") % db["sex"];  sbuf+=buf;
      buf = format("spec_proc: %s\n") % db["spec_proc"];  sbuf+=buf;
      buf = format("skin: %s\n") % db["skin"];  sbuf+=buf;
      buf = format("vision: %s\n") % db["vision"];  sbuf+=buf;
      buf = format("can_be_seen: %s\n") % db["can_be_seen"];  sbuf+=buf;
      buf = format("max_exist: %s\n") % db["max_exist"];  sbuf+=buf;
      buf = format("local_sound~:\n%s~\n") % db["local_sound"];  sbuf+=buf;
      buf = format("adjacent_sound~:\n%s~\n") % db["adjacent_sound"];  sbuf+=buf;


      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from mob_extra where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- mob_extra\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("keyword: %s\n") % db["keyword"];  sbuf+=buf;
      buf = format("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = format("\n"); sbuf+=buf;
    }


    db.query("select * from mob_imm where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = format("- mob_imm\n"); sbuf+=buf;
      buf = format("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = format("type: %s\n") % db["type"];  sbuf+=buf;
      buf = format("amt: %s\n") % db["amt"];  sbuf+=buf;
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


  printf("Insert mob%s? [y/n] ", vnums.size()>1?"s":"");
  while(1){
    answer=getc(stdin);
    if(answer=='n'){
      printf("Mob%s not inserted.\n", vnums.size()>1?"s":"");
      unlink(file);
      exit(0);
    } else if(answer=='y'){
      break;
    } else {
      printf("Insert mob%s? [y/n] ", vnums.size()>1?"s":"");
    }
  }

  buf = format("/bin/cp -f %s %s.backup") % file % file;
  system(buf.c_str());


  int i=0;

  while(++i){
    val=parse_data_file(file, i);
    if(val["vnum"]=="EOM")
      break;
    
    if(val["DATATYPE"]=="mob"){
      printf("replacing mob %s\n", val["vnum"].c_str());
      db.query("delete from mob where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from mob_extra where vnum=%s",
	       val["vnum"].c_str());
      db.query("delete from mob_imm where vnum=%s",
	       val["vnum"].c_str());

      if(val["long_desc"][val["long_desc"].size()-1] == '\n'){
	val["long_desc"]+="\r";
      }

      if(val["long_desc"][val["long_desc"].size()-1] != '\r' &&
	 val["long_desc"][val["long_desc"].size()-2] != '\n'){
	val["long_desc"]+="\n\r";
      }
      if(val["description"][val["description"].size()-1] == '\n'){
	val["description"]+="\r";
      }

      if(val["description"][val["description"].size()-1] != '\r' &&
	 val["description"][val["description"].size()-2] != '\n'){
	val["description"]+="\n\r";
      }

      db.query("insert into mob (vnum,name,short_desc,long_desc,description,actions,affects,faction,fact_perc,letter,attacks,class,level,tohit,ac,hpbonus,damage_level,damage_precision,gold,race,weight,height,str,bra,con,dex,agi,intel,wis,foc,per,cha,kar,spe,pos,def_position,sex,spec_proc,skin,vision,can_be_seen,max_exist,local_sound,adjacent_sound) values (%s,'%s','%s','%s','%s',%s, %s, %s, %s, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%s', '%s')",
	       val["vnum"].c_str(),val["name"].c_str(),val["short_desc"].c_str(),val["long_desc"].c_str(),val["description"].c_str(),val["actions"].c_str(),val["affects"].c_str(),val["faction"].c_str(),val["fact_perc"].c_str(),val["letter"].c_str(),val["attacks"].c_str(),val["class"].c_str(),val["level"].c_str(),val["tohit"].c_str(),val["ac"].c_str(),val["hpbonus"].c_str(),val["damage_level"].c_str(),val["damage_precision"].c_str(),val["gold"].c_str(),val["race"].c_str(),val["weight"].c_str(),val["height"].c_str(),val["str"].c_str(),val["bra"].c_str(),val["con"].c_str(),val["dex"].c_str(),val["agi"].c_str(),val["intel"].c_str(),val["wis"].c_str(),val["foc"].c_str(),val["per"].c_str(),val["cha"].c_str(),val["kar"].c_str(),val["spe"].c_str(),val["pos"].c_str(),val["def_position"].c_str(),val["sex"].c_str(),val["spec_proc"].c_str(),val["skin"].c_str(),val["vision"].c_str(),val["can_be_seen"].c_str(),val["max_exist"].c_str(),val["local_sound"].c_str(),val["adjacent_sound"].c_str());
      
    } else if(val["DATATYPE"]=="mob_extra"){
      printf("replacing mob_extra %s\n", val["vnum"].c_str());
      
      db.query("insert into mob_extra (vnum, keyword, description) values (%s,'%s','%s')", val["vnum"].c_str(), val["keyword"].c_str(), val["description"].c_str());
      
    } else if(val["DATATYPE"]=="mob_imm"){
      printf("replacing mob_imm %s\n", val["vnum"].c_str());
      
      db.query("insert into mob_imm (vnum,type,amt) values (%s,%s,%s)",
	       val["vnum"].c_str(), val["type"].c_str(),val["amt"].c_str());

    }
    printf("\n");

  }


  unlink(file);

  printf("Done.\n\r");
  printf("Your backup file is %s.backup\n", file);
}
