#include "stdsneezy.h"
#include "database.h"
#include "lowtools.h"

int main(int argc, char **argv)
{
  TDatabase db_immo(DB_IMMORTAL);
  TDatabase db_beta(DB_SNEEZYBETA);
  sstring immortal;
  vector<int>vnums;
  int actions;

  toggleInfo.loadToggles();
  
  if((argc-1) < 2){
    printf("Usage: %s <immortal> <mob list>\n", argv[0]);;
    exit(0);
  }

  immortal=argv[1];
  
  if(!parse_num_args(argc-2, argv+2, vnums))
    exit(0);

  printf("Processing mobs for %s\n", immortal.c_str());

  // loop through item nums
  for(unsigned int t=0;t<vnums.size();t++){
    //// mob
    db_immo.query("select vnum, name, short_desc, long_desc, description, actions, affects, faction, fact_perc, letter, attacks, class, level, tohit, ac, hpbonus, damage_level, damage_precision, gold, race, weight, height, str, bra, con, dex, agi, intel, wis, foc, per, cha, kar, spe, pos, def_position, sex, spec_proc, skin, vision, can_be_seen, max_exist, local_sound, adjacent_sound from mob where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);
    
    if(db_immo.fetchRow()){
      printf("Adding %i ('%s')\n", vnums[t], db_immo["short_desc"].c_str());
      
      // fix strung bit
      actions=convertTo<int>(db_immo["actions"]);
      if (actions & 1<<0)
        actions = actions & ~1<<0;
      
      db_beta.query("delete from mob where vnum=%i", vnums[t]);
      db_beta.query("insert into mob values(%s, '%s', '%s', '%s', '%s', %i, %s, %s, %s, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%s', '%s')",
          db_immo["vnum"].c_str(), db_immo["name"].c_str(), db_immo["short_desc"].c_str(), 
          db_immo["long_desc"].c_str(), db_immo["description"].c_str(), actions, 
          db_immo["affects"].c_str(), db_immo["faction"].c_str(), db_immo["fact_perc"].c_str(), 
          db_immo["letter"].c_str(), db_immo["attacks"].c_str(), db_immo["class"].c_str(), 
          db_immo["level"].c_str(), db_immo["tohit"].c_str(), db_immo["ac"].c_str(), 
          db_immo["hpbonus"].c_str(), db_immo["damage_level"].c_str(), db_immo["damage_precision"].c_str(), 
          db_immo["gold"].c_str(), db_immo["race"].c_str(), db_immo["weight"].c_str(), 
          db_immo["height"].c_str(), 
          db_immo["str"].c_str(), db_immo["bra"].c_str(), db_immo["con"].c_str(), 
          db_immo["dex"].c_str(), db_immo["agi"].c_str(), db_immo["intel"].c_str(), 
          db_immo["wis"].c_str(), db_immo["foc"].c_str(), db_immo["per"].c_str(), 
          db_immo["cha"].c_str(), db_immo["kar"].c_str(), db_immo["spe"].c_str(), 
          db_immo["pos"].c_str(), db_immo["def_position"].c_str(), db_immo["sex"].c_str(), 
          db_immo["spec_proc"].c_str(), db_immo["skin"].c_str(), db_immo["vision"].c_str(), 
          db_immo["can_be_seen"].c_str(), db_immo["max_exist"].c_str(), 
          db_immo["local_sound"].c_str(), db_immo["adjacent_sound"].c_str());


      //// mob_imm
      db_beta.query("delete from mob_imm where vnum=%i", vnums[t]);

      db_immo.query("select vnum, type, amt from mob_imm where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);

      while(db_immo.fetchRow()){
        db_beta.query("insert into mob_imm values(%s, %s, %s)", db_immo["vnum"].c_str(), db_immo["type"].c_str(), db_immo["amt"].c_str());
      }      

    
      //// mob_extra
      db_beta.query("delete from mob_extra where vnum=%i", vnums[t]);

      db_immo.query("select vnum, keyword, description from mob_extra where owner='%s' and vnum=%i", immortal.c_str(), vnums[t]);
      
      while(db_immo.fetchRow()){
        db_beta.query("insert into mob_extra values(%s, '%s', '%s')", db_immo["vnum"].c_str(), db_immo["keyword"].c_str(), db_immo["description"].c_str());
      }

    } else {
      printf("Not found: %i\n", vnums[t]);
    }

  }

}



