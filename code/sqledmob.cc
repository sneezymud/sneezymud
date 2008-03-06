#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"
#include <unistd.h>

int main(int argc, char **argv)
{
  TDatabase db(DB_SNEEZYBETA);
  vector<int>vnums;
  map<sstring,sstring>val;
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
      buf = fmt("- mob\n"); sbuf+=buf;
      buf = fmt("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = fmt("name: %s\n") % db["name"];  sbuf+=buf;
      buf = fmt("short_desc: %s\n") % db["short_desc"];  sbuf+=buf;
      buf = fmt("long_desc~:\n%s~\n") % db["long_desc"];  sbuf+=buf;
      buf = fmt("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = fmt("actions: %s\n") % db["actions"];  sbuf+=buf;
      buf = fmt("affects: %s\n") % db["affects"];  sbuf+=buf;
      buf = fmt("faction: %s\n") % db["faction"];  sbuf+=buf;
      buf = fmt("fact_perc: %s\n") % db["fact_perc"];  sbuf+=buf;
      buf = fmt("letter: %s\n") % db["letter"];  sbuf+=buf;
      buf = fmt("attacks: %s\n") % db["attacks"];  sbuf+=buf;
      buf = fmt("class: %s\n") % db["class"];  sbuf+=buf;
      buf = fmt("level: %s\n") % db["level"];  sbuf+=buf;
      buf = fmt("tohit: %s\n") % db["tohit"];  sbuf+=buf;
      buf = fmt("ac: %s\n") % db["ac"];  sbuf+=buf;
      buf = fmt("hpbonus: %s\n") % db["hpbonus"];  sbuf+=buf;
      buf = fmt("damage_level: %s\n") % db["damage_level"];  sbuf+=buf;
      buf = fmt("damage_precision: %s\n") % db["damage_precision"];  sbuf+=buf;
      buf = fmt("gold: %s\n") % db["gold"];  sbuf+=buf;
      buf = fmt("race: %s\n") % db["race"];  sbuf+=buf;
      buf = fmt("weight: %s\n") % db["weight"];  sbuf+=buf;
      buf = fmt("height: %s\n") % db["height"];  sbuf+=buf;
      buf = fmt("str: %s\n") % db["str"];  sbuf+=buf;
      buf = fmt("bra: %s\n") % db["bra"];  sbuf+=buf;
      buf = fmt("con: %s\n") % db["con"];  sbuf+=buf;
      buf = fmt("dex: %s\n") % db["dex"];  sbuf+=buf;
      buf = fmt("agi: %s\n") % db["agi"];  sbuf+=buf;
      buf = fmt("intel: %s\n") % db["intel"];  sbuf+=buf;
      buf = fmt("wis: %s\n") % db["wis"];  sbuf+=buf;
      buf = fmt("foc: %s\n") % db["foc"];  sbuf+=buf;
      buf = fmt("per: %s\n") % db["per"];  sbuf+=buf;
      buf = fmt("cha: %s\n") % db["cha"];  sbuf+=buf;
      buf = fmt("kar: %s\n") % db["kar"];  sbuf+=buf;
      buf = fmt("spe: %s\n") % db["spe"];  sbuf+=buf;
      buf = fmt("pos: %s\n") % db["pos"];  sbuf+=buf;
      buf = fmt("def_position: %s\n") % db["def_position"];  sbuf+=buf;
      buf = fmt("sex: %s\n") % db["sex"];  sbuf+=buf;
      buf = fmt("spec_proc: %s\n") % db["spec_proc"];  sbuf+=buf;
      buf = fmt("skin: %s\n") % db["skin"];  sbuf+=buf;
      buf = fmt("vision: %s\n") % db["vision"];  sbuf+=buf;
      buf = fmt("can_be_seen: %s\n") % db["can_be_seen"];  sbuf+=buf;
      buf = fmt("max_exist: %s\n") % db["max_exist"];  sbuf+=buf;
      buf = fmt("local_sound~:\n%s~\n") % db["local_sound"];  sbuf+=buf;
      buf = fmt("adjacent_sound~:\n%s~\n") % db["adjacent_sound"];  sbuf+=buf;


      buf = fmt("\n"); sbuf+=buf;
    }


    db.query("select * from mob_extra where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = fmt("- mob_extra\n"); sbuf+=buf;
      buf = fmt("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = fmt("keyword: %s\n") % db["keyword"];  sbuf+=buf;
      buf = fmt("description~:\n%s~\n") % db["description"];  sbuf+=buf;
      buf = fmt("\n"); sbuf+=buf;
    }


    db.query("select * from mob_imm where vnum=%i", vnums[i]);
    while(db.fetchRow()){
      buf = fmt("- mob_imm\n"); sbuf+=buf;
      buf = fmt("vnum: %s\n") % db["vnum"];  sbuf+=buf;
      buf = fmt("type: %s\n") % db["type"];  sbuf+=buf;
      buf = fmt("amt: %s\n") % db["amt"];  sbuf+=buf;
      buf = fmt("\n"); sbuf+=buf;
    }
  }


  mkstemp(file);
  tmpfile=fopen(file, "w");
  fprintf(tmpfile, "%s", sbuf.c_str());
  fclose(tmpfile);

  printf("Opening editor.\n");
  buf = fmt("$EDITOR %s") % file;
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

  buf = fmt("/bin/cp -f %s %s.backup") % file % file;
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
