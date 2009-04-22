#include <iostream>
#include <fstream>

#include "configuration.h"
#include "extern.h"
#include "database.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// static data member defs
int Config::ITEM_DAMAGE_RATE;
int Config::RENT_CREDIT_VAL;
bool Config::RENT_SELL_TO_PAWN;
bool Config::RENT_RESTRICT_INNS_BY_LEVEL;
bool Config::PENALIZE_FOR_AUTO_RENTING;
int Config::WEAPON_DAM_MIN_HARDNESS;
int Config::WEAPON_DAM_MAX_HARDNESS;
int Config::WEAPON_DAM_MAX_SHARP;
bool Config::SPEEF_MAKE_BODY;
bool Config::NUKE_REPAIR_ITEMS;
bool Config::CHECK_MULTIPLAY;
bool Config::FORCE_MULTIPLAY_COMPLIANCE;
bool Config::REPO_MOBS;
bool Config::SUPER_REPO_MOBS;
bool Config::NO_DAMAGED_ITEMS_SHOP;
bool Config::auto_deletion;
bool Config::rent_only_deletion;
bool Config::nuke_inactive_mobs;
bool Config::load_on_death;

const int PROD_GAMEPORT = 7900;
const int PROD_XMLPORT = 7901;
const int BETA_GAMEPORT = 5678;
const int ALPHA_GAMEPORT = 6969;
const int BUILDER_GAMEPORT = 8900;
      int GAMMA_GAMEPORT = 6961; // Maror - quick boot! (skips zones) -Updated to allow otf swapping -Lapsos

void sendHelp(po::options_description desc){
  std::cout << "Usage: sneezy [options] [port]" << std::endl;
  std::cout << desc;  
}

bool Config::doConfiguration(int argc, char *argv[])
{
  string configFile="sneezy.cfg";

  // command line only options
  po::options_description cmdline("Command line only");
  cmdline.add_options()
    ("help", "produce help message")
    ("config,c", po::value<string>(&configFile)->default_value("sneezy.cfg"),
     "configuration file to use")
    ;

  // command line OR in config file
  po::options_description config("Configuration + Command line");
  config.add_options()
    ("lib,l", po::value<string>(&dir)->default_value(DFLT_DIR), 
     "data directory to run in")
    ("nospecials,s", po::value<bool>(&noSpecials)->zero_tokens(),
     "suppress assignment of special routines")
    ("trimmed,t", po::value<bool>(&bTrimmed)->zero_tokens(),
     "load as trimmed port")
    ("port,p", po::value<int>(&gamePort)->default_value(PROD_GAMEPORT),
     "game port")
    ;

  // config file only options
  po::options_description configOnly("Configuration File Only");
  configOnly.add_options()
    ("item_damage_rate", 
     po::value<int>(&ITEM_DAMAGE_RATE)->default_value(1),
     "see configuration.h")
    ("rent_credit_val",
     po::value<int>(&RENT_CREDIT_VAL)->default_value(75),
     "see configuration.h")
    ("rent_sell_to_pawn",
     po::value<bool>(&RENT_SELL_TO_PAWN)->default_value(false),
     "see configuration.h")
    ("rent_restrict_inns_by_level",
     po::value<bool>(&RENT_RESTRICT_INNS_BY_LEVEL)->default_value(false),
     "see configuration.h")
    ("penalize_for_auto_renting",
     po::value<bool>(&PENALIZE_FOR_AUTO_RENTING)->default_value(true),
     "see configuration.h")
    ("weapon_dam_min_hardness",
     po::value<int>(&WEAPON_DAM_MIN_HARDNESS)->default_value(20),
     "see configuration.h")
    ("weapon_dam_max_hardness",
     po::value<int>(&WEAPON_DAM_MAX_HARDNESS)->default_value(150),
     "see configuration.h")
    ("weapon_dam_max_sharp",
     po::value<int>(&WEAPON_DAM_MAX_SHARP)->default_value(150),
     "see configuration.h")
    ("speef_make_body",
     po::value<bool>(&SPEEF_MAKE_BODY)->default_value(false),
     "see configuration.h")
    ("nuke_repair_items",
     po::value<bool>(&NUKE_REPAIR_ITEMS)->default_value(true),
     "see configuration.h")
    ("check_multiplay",
     po::value<bool>(&CHECK_MULTIPLAY)->default_value(true),
     "see configuration.h")
    ("force_multiplay_compliance",
     po::value<bool>(&FORCE_MULTIPLAY_COMPLIANCE)->default_value(true),
     "see configuration.h")
    ("repo_mobs",
     po::value<bool>(&REPO_MOBS)->default_value(false),
     "see configuration.h")
    ("super_repo_mobs",
     po::value<bool>(&SUPER_REPO_MOBS)->default_value(false),
     "see configuration.h")
    ("no_damaged_items_shop",
     po::value<bool>(&NO_DAMAGED_ITEMS_SHOP)->default_value(false),
     "see configuration.h")
    ("auto_deletion",
     po::value<bool>(&auto_deletion)->default_value(false),
     "see configuration.h")
    ("rent_only_deletion",
     po::value<bool>(&rent_only_deletion)->default_value(false),
     "see configuration.h")
    ("nuke_inactive_mobs",
     po::value<bool>(&nuke_inactive_mobs)->default_value(false),
     "see configuration.h")
    ("load_on_death",
     po::value<bool>(&load_on_death)->default_value(false),
     "see configuration.h")
    ;

  // database options
  po::options_description databases("Databases");
  databases.add_options()
    ("sneezy_db", po::value<string>(&db_hosts[DB_SNEEZY]),
     "host for sneezy database")
    ("sneezybeta_db", po::value<string>(&db_hosts[DB_SNEEZYBETA]),
     "host for sneezybeta database (unused)")
    ("immortal_db", po::value<string>(&db_hosts[DB_IMMORTAL]),
     "host for immortal database")
    ("sneezyglobal_db", po::value<string>(&db_hosts[DB_SNEEZYGLOBAL]),
     "host for sneezyglobal database")
    ("sneezyprod_db", po::value<string>(&db_hosts[DB_SNEEZYPROD]),
     "host for sneezyprod database (unused)")
    ("sneezybuilder_db", po::value<string>(&db_hosts[DB_SNEEZYBUILDER]),
     "host for sneezybuilder database (unused)")
    ("wiki_mortal_db", po::value<string>(&db_hosts[DB_WIKI_MORTAL]),
     "host for mortal wiki database")
    ("wiki_builder_db", po::value<string>(&db_hosts[DB_WIKI_BUILDER]),
     "host for builder wiki database")
    ("wiki_admin_db", po::value<string>(&db_hosts[DB_WIKI_ADMIN]),
     "host for admin wiki database")
    ("forums_admin_db", po::value<string>(&db_hosts[DB_FORUMS_ADMIN]),
     "host for admin forums database")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(cmdline).add(config).add(databases);

  po::options_description config_options;
  config_options.add(config).add(databases);

  po::options_description config_only_options;
  config_only_options.add(configOnly);

  po::options_description visible("Allowed options");
  visible.add(cmdline).add(config).add(databases).add(configOnly);


  // first positional argument is port number
  po::positional_options_description p;
  p.add("port", -1);
  
  po::variables_map vm;


  try {
    if(argc){
      po::store(po::command_line_parser(argc, argv).
		options(cmdline_options).positional(p).run(), vm);
    }
    po::notify(vm);
    std::ifstream ifs(configFile.c_str());

    po::store(parse_config_file(ifs, config_options), vm);
    po::store(parse_config_file(ifs, config_only_options), vm);
    po::notify(vm);
  } catch(po::unknown_option){
    sendHelp(visible);
    return false;    
  }

  if(vm.count("help")){
    sendHelp(visible);
    return false;
  }
  return true;
}


