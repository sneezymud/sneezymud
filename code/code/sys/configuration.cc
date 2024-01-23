#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>

#include "configuration.h"
#include "extern.h"
#include "database.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// static data member defs
int Config::ITEM_DAMAGE_RATE;
int Config::WEAPON_DAM_MIN_HARDNESS;
int Config::WEAPON_DAM_MAX_HARDNESS;
int Config::WEAPON_DAM_MAX_SHARP;
bool Config::NUKE_REPAIR_ITEMS;
bool Config::CHECK_MULTIPLAY;
bool Config::FORCE_MULTIPLAY_COMPLIANCE;
bool Config::REPO_MOBS;
bool Config::SUPER_REPO_MOBS;
bool Config::NO_DAMAGED_ITEMS_SHOP;
bool Config::auto_deletion;
bool Config::rent_only_deletion;
bool Config::rent_tax;
bool Config::nuke_inactive_mobs;
bool Config::load_on_death;
bool Config::throw_format_exceptions;
bool Config::no_specials;
bool Config::b_trimmed;
sstring Config::data_dir;
bool Config::no_mail;

const int Config::Port::PROD = 7900;
const int Config::Port::ALPHA = 6969;
const int Config::Port::GAMMA = 6961;

extern std::vector<std::string> db_hosts;
extern std::vector<std::string> db_names;
extern std::vector<std::string> db_users;
extern std::vector<std::string> db_passwords;

void sendHelp(po::options_description desc) {
  std::cout << "Usage: sneezy [options] [port]" << std::endl;
  std::cout << desc;
}

bool Config::doConfiguration(int argc, char* argv[]) {
  using std::string;
  string configFile = "sneezy.cfg";

  // command line only options
  po::options_description cmdline("Command line only");
  cmdline.add_options()("help", "produce help message")("config,c",
    po::value<string>(&configFile)->default_value("sneezy.cfg"),
    "configuration file to use");

  // command line OR in config file
  po::options_description config("Configuration + Command line");
  config.add_options()("lib,l",
    po::value<string>(&data_dir)->default_value(Path::DATA),
    "data directory to run in")("no_specials,s",
    po::value<bool>(&no_specials)->default_value(false),
    "suppress assignment of special routines")("trimmed,t",
    po::value<bool>(&b_trimmed)->default_value(false),
    "load as trimmed port")("port,p",
    po::value<int>(&gamePort)->default_value(Config::Port::PROD), "game port");

  // config file only options
  po::options_description configOnly("Configuration File Only");
  configOnly.add_options()("item_damage_rate",
    po::value<int>(&ITEM_DAMAGE_RATE)->default_value(1),
    "see configuration.h")("weapon_dam_min_hardness",
    po::value<int>(&WEAPON_DAM_MIN_HARDNESS)->default_value(20),
    "see configuration.h")("weapon_dam_max_hardness",
    po::value<int>(&WEAPON_DAM_MAX_HARDNESS)->default_value(150),
    "see configuration.h")("weapon_dam_max_sharp",
    po::value<int>(&WEAPON_DAM_MAX_SHARP)->default_value(150),
    "see configuration.h")("nuke_repair_items",
    po::value<bool>(&NUKE_REPAIR_ITEMS)->default_value(true),
    "see configuration.h")("check_multiplay",
    po::value<bool>(&CHECK_MULTIPLAY)->default_value(true),
    "see configuration.h")("force_multiplay_compliance",
    po::value<bool>(&FORCE_MULTIPLAY_COMPLIANCE)->default_value(true),
    "see configuration.h")("repo_mobs",
    po::value<bool>(&REPO_MOBS)->default_value(false), "see configuration.h")(
    "super_repo_mobs", po::value<bool>(&SUPER_REPO_MOBS)->default_value(false),
    "see configuration.h")("no_damaged_items_shop",
    po::value<bool>(&NO_DAMAGED_ITEMS_SHOP)->default_value(false),
    "see configuration.h")("auto_deletion",
    po::value<bool>(&auto_deletion)->default_value(false),
    "see configuration.h")("rent_only_deletion",
    po::value<bool>(&rent_only_deletion)->default_value(false),
    "see configuration.h")("rent_tax",
    po::value<bool>(&rent_tax)->default_value(true),
    "see configuration.h")("nuke_inactive_mobs",
    po::value<bool>(&nuke_inactive_mobs)->default_value(false),
    "see configuration.h")("load_on_death",
    po::value<bool>(&load_on_death)->default_value(true),
    "see configuration.h")("throw_format_exceptions",
    po::value<bool>(&throw_format_exceptions)->default_value(true),
    "see configuration.h")("no_mail",
    po::value<bool>(&no_mail)->default_value(false), "see configuration.h");

  // database options
  po::options_description database_hosts("Database hosts");
  database_hosts.add_options()("sneezy_host",
    po::value<string>(&db_hosts[DB_SNEEZY]),
    "host for sneezy database")("immortal_host",
    po::value<string>(&db_hosts[DB_IMMORTAL]), "host for immortal database");

  // database options
  po::options_description database_names("Database names");
  database_names.add_options()("sneezy_db",
    po::value<string>(&db_names[DB_SNEEZY]),
    "name for sneezy database")("immortal_db",
    po::value<string>(&db_names[DB_IMMORTAL]), "name for immortal database")

    ;

  po::options_description database_users("Database usernames");
  database_users.add_options()("sneezy_user",
    po::value<string>(&db_users[DB_SNEEZY]),
    "sneezy database username")("immortal_user",
    po::value<string>(&db_users[DB_IMMORTAL]), "immortal database username");

  po::options_description database_passwords("Database passwords");
  database_users.add_options()("sneezy_password",
    po::value<string>(&db_passwords[DB_SNEEZY]), "sneezy database password")(
    "immortal_password", po::value<string>(&db_passwords[DB_IMMORTAL]),
    "immortal database password");

  po::options_description cmdline_options;
  cmdline_options.add(cmdline)
    .add(config)
    .add(database_names)
    .add(database_hosts)
    .add(database_users)
    .add(database_passwords);

  po::options_description config_options;
  config_options.add(config)
    .add(database_names)
    .add(database_hosts)
    .add(database_users)
    .add(database_passwords)
    .add(configOnly);

  po::options_description visible("Allowed options");
  visible.add(cmdline)
    .add(config)
    .add(database_names)
    .add(database_hosts)
    .add(database_users)
    .add(database_passwords)
    .add(configOnly);

  // first positional argument is port number
  po::positional_options_description p;
  p.add("port", -1);

  po::variables_map vm;

  if (argc) {
    po::store(po::command_line_parser(argc, argv)
                .options(cmdline_options)
                .positional(p)
                .run(),
      vm);
  }
  po::notify(vm);
  std::ifstream ifs(configFile.c_str());

  if (!ifs.is_open()) {
    std::cout << format("Failed to open config file '%s'\n") % configFile;
  }

  po::store(parse_config_file(ifs, config_options), vm);
  po::notify(vm);

  if (vm.count("help")) {
    sendHelp(visible);
    return false;
  }
  return true;
}

std::string readFileContents(const std::string& path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    vlogf(LOG_FILE,
      format("Failed to open %s.") % (std::filesystem::current_path() / path));
    return {};
  }

  return {(std::istreambuf_iterator<char>(file)), {}};
}

std::string readVersionFromFile() {
  const std::filesystem::path versionPath =
    std::filesystem::current_path() / Config::DataDir().c_str() / "version.txt";

  std::istringstream fileContents(readFileContents(versionPath.string()));

  std::string hash;
  std::getline(fileContents, hash);

  std::string date;
  std::getline(fileContents, date);

  return {"(build " + hash + ", " + date + ")"};
}
