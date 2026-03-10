#include "database.h"
#include "log.h"
#include "migrations.h"

#include "charfile.h"
#include "player_data.h"

#include <cassert>
#include <boost/format.hpp>
#include <map>

namespace {
  int getVersion(TDatabase& sneezy) {
    sneezy.query("select value from configuration where config = 'version'");
    if (sneezy.fetchRow())
      return stoi(sneezy["value"]);
    return 0;
  }
}  // namespace

void runMigrations() {
  TDatabase sneezy(DB_SNEEZY, true);
  TDatabase immortal(DB_IMMORTAL, true);

  std::vector<std::function<void()>> migrations = {
    [&]() {
      vlogf(LOG_MISC, "Adding configuration table");
      assert(sneezy.query(
        "create table configuration (id int primary key auto_increment not "
        "null, config varchar(100) unique not null, value varchar(999) null)"));
      assert(sneezy.query(
        "create unique index idx_configuration_key on configuration (config)"));
      assert(sneezy.query(
        "insert into configuration (config, value) values ('version', '0')"));
    },
    [&]() {
      vlogf(LOG_MISC, "Migrating aliases to DB");
      assert(sneezy.query(
        "create table alias ("
        "id int primary key auto_increment not null, "
        "player_id bigint(20) unsigned not null, "
        "word varchar(50) not null, "
        "command varchar(999) not null, "
        "foreign key (player_id) references player (id) on delete cascade)"));

      assert(sneezy.query("select id, name from player"));
      std::map<int, std::string> idToName;
      while (sneezy.fetchRow())
        idToName[convertTo<int>(sneezy["id"])] = sneezy["name"];

      for (auto& player : idToName) {
        const auto id = player.first;
        const auto& name = player.second;
        charFile file;
        if (!load_char(name, &file)) {
          vlogf(LOG_MISC, format("Cannot open player file for %s") % name);
          continue;
        }

        for (auto alias : file.alias) {
          if (alias.word[0]) {
            vlogf(LOG_MISC, format("%d/%s: %s -> %s") % id % name % alias.word %
                              alias.command);
            assert(
              sneezy.query("insert into alias (player_id, word, command) "
                           "values (%i, '%s', '%s')",
                id, alias.word, alias.command));
          }
        }
      }
    },
    [&]() {
      // support tweaks db
      vlogf(LOG_MISC, "Adding tweak table to DB");
      assert(sneezy.query(
        "create table globaltweaks ("
        "tweak_id int primary key auto_increment not null, "
        "tweak_type int not null, "
        "tweak_value float(20) not null, "
        "tweak_target float(20) not null, "
        "tweak_rate float(20) not null, "
        "datecreated datetime not null default CURRENT_TIMESTAMP)"));
    },
    [&]() {
      // configurable multiplay limit per account
      vlogf(LOG_MISC, "Adding multiplay column to account table");
      assert(
        sneezy.query("alter table account "
                     "add column multiplay_limit int null default 2"));
      assert(
        sneezy.query("update account "
                     "set multiplay_limit = 3"));
      assert(
        sneezy.query("alter table account "
                     "modify column multiplay_limit int not null default 2"));
    },
    [&]() {
      vlogf(LOG_MISC, "Renaming Test Code 6 into DB Logging");
      assert(sneezy.query(
        "update globaltoggles "
        "set name = 'DB Logging', descr = 'log all db queries', testcode = 0 "
        "where tog_id = 17"));
    },
    [&]() {
      vlogf(LOG_MISC, "Adding savedrooms table");
      assert(sneezy.query(
        "create table savedrooms ("
        "id int primary key auto_increment not null, "
        "player_id bigint(20) unsigned not null, "
        "name varchar(50) not null, "
        "room int not null, "
        "foreign key (player_id) references player (id) on delete cascade)"));
    },
    [&]() {
      vlogf(LOG_MISC, "Tying saved rooms to accounts");
      assert(sneezy.query("drop table if exists savedroomsacct"));
      assert(
        sneezy.query("create table savedroomsacct ("
                     "id int primary key auto_increment not null, "
                     "account_id bigint(20) unsigned not null, "
                     "name varchar(50) not null, "
                     "room int not null, "
                     "foreign key (account_id) references account (account_id) "
                     "on delete cascade)"));
      assert(
        sneezy.query("insert into savedroomsacct select "
                     "s.id, a.account_id, s.name, s.room "
                     "from savedrooms s join player p "
                     "on s.player_id = p.id "
                     "join account a on p.account_id = a.account_id "));
      assert(sneezy.query("drop table savedrooms"));
    },
    [&]() {
      vlogf(LOG_MISC, "Adding generic per-account and per-player storage");
      assert(
        sneezy.query("create table if not exists accountnotes ("
                     "id int primary key auto_increment not null, "
                     "account_id bigint(20) unsigned not null, "
                     "name varchar(64) not null, "
                     "value text not null, "
                     "foreign key (account_id) references account (account_id) "
                     "on delete cascade)"));
      assert(sneezy.query(
        "create table if not exists playernotes ("
        "id int primary key auto_increment not null, "
        "player_id bigint(20) unsigned not null, "
        "name varchar(64) not null, "
        "value text not null, "
        "foreign key (player_id) references player (id) on delete cascade)"));
    },
    [&]() {
      vlogf(LOG_MISC, "Moving wiz data over to db");
      assert(sneezy.query(
        "create table if not exists wizdata ("
        "setsev int not null,"
        "office int default 0,"
        "blockastart int,"
        "blockaend int,"
        "blockbstart int,"
        "blockbend int,"
        "player_id bigint(20) unsigned not null, "
        "primary key (player_id), "
        "foreign key (player_id) references player (id) on delete cascade)"));

      class wizSaveData {
        public:
          int setsev, office, blockastart, blockaend, blockbstart, blockbend;
      };

      assert(sneezy.query("select id, name from player"));
      while (sneezy.fetchRow()) {
        // The db contains lowercased names and /immortals uses CamelCase. First
        // we have to use the lower case name from the db to open the charfile
        // to extract the CamelCase name to get to the immortals wizdata file
        charFile file;
        if (!load_char(sneezy["name"], &file)) {
          vlogf(LOG_MISC,
            format("WizData migration: Cannot open player file for %s") %
              sneezy["name"]);
          continue;
        }

        FILE* fp;
        sstring buf;
        wizSaveData saveData;

        buf = format("mutable/immortals/%s/wizdata") % file.name;
        fp = fopen(buf.c_str(), "r");
        if (!fp) {
          continue;
        }
        if (fread(&saveData, sizeof(saveData), 1, fp) != 1) {
          vlogf(LOG_BUG,
            format("Corrupt wiz save file for %s") % sneezy["name"]);
          fclose(fp);
          continue;
        }

        fclose(fp);
        assert(
          sneezy.query("insert into wizdata (setsev, office, blockastart, "
                       "blockaend, blockbstart, blockbend, player_id) "
                       "values (%i, %i, %i,%i, %i, %i, %i)",
            saveData.setsev, saveData.office, saveData.blockastart,
            saveData.blockaend, saveData.blockbstart, saveData.blockbend,
            convertTo<int>(sneezy["id"])));
      }
    },
    [&]() {
      vlogf(LOG_MISC, "Adding PKs, indexes, and FK cascades to shop tables");

      // Indexes on parent tables
      assert(
        sneezy.query("ALTER TABLE shop ADD INDEX idx_shop_keeper (keeper)"));
      assert(
        sneezy.query("ALTER TABLE shop ADD INDEX idx_shop_in_room (in_room)"));
      assert(sneezy.query(
        "ALTER TABLE shopowned ADD INDEX idx_shopowned_corp_id (corp_id)"));
      assert(sneezy.query(
        "ALTER TABLE shopowned ADD INDEX idx_shopowned_tax_nr (tax_nr)"));

      // Primary keys on direct shop children
      assert(
        sneezy.query("ALTER TABLE shoptype ADD PRIMARY KEY (shop_nr, type)"));
      assert(sneezy.query(
        "ALTER TABLE shopproducing ADD PRIMARY KEY (shop_nr, producing)"));
      assert(sneezy.query(
        "ALTER TABLE shopmaterial ADD PRIMARY KEY (shop_nr, mat_type)"));

      // FK cascades: direct shop children -> shop
      assert(
        sneezy.query("ALTER TABLE shoptype ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shop (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopproducing ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shop (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopmaterial ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shop (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopowned ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shop (shop_nr) ON DELETE CASCADE"));

      // FK: shopowned.corp_id -> corporation (SET NULL on corp delete)
      // corp_id must match corporation.corp_id's type (bigint unsigned)
      assert(sneezy.query(
        "ALTER TABLE shopowned "
        "MODIFY corp_id bigint(20) unsigned NULL"));
      assert(
        sneezy.query("ALTER TABLE shopowned ADD FOREIGN KEY (corp_id) "
                     "REFERENCES corporation (corp_id) ON DELETE SET NULL"));

      // Fix nullable columns on shopowned children, then add PKs
      // shopownedbank
      assert(
        sneezy.query("ALTER TABLE shopownedbank "
                     "MODIFY shop_nr int NOT NULL, "
                     "MODIFY player_id int NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr, player_id)"));
      // shopownedcorpbank
      assert(
        sneezy.query("ALTER TABLE shopownedcorpbank "
                     "MODIFY shop_nr int NOT NULL, "
                     "MODIFY corp_id int NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr, corp_id)"));
      // shopownedloanrate
      assert(
        sneezy.query("ALTER TABLE shopownedloanrate "
                     "MODIFY shop_nr int NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr)"));
      // shopownedmatch
      assert(
        sneezy.query("ALTER TABLE shopownedmatch "
                     "MODIFY shop_nr int NOT NULL, "
                     "MODIFY match_str varchar(255) NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr, match_str)"));
      // shopownedplayer
      assert(
        sneezy.query("ALTER TABLE shopownedplayer "
                     "MODIFY shop_nr int NOT NULL, "
                     "MODIFY player varchar(80) NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr, player)"));
      // shopownedrepair
      assert(
        sneezy.query("ALTER TABLE shopownedrepair "
                     "MODIFY shop_nr int NOT NULL, "
                     "ADD PRIMARY KEY (shop_nr)"));
      // shopownedaccess and shopownedratios are already NOT NULL
      assert(sneezy.query(
        "ALTER TABLE shopownedaccess ADD PRIMARY KEY (shop_nr, name)"));
      assert(sneezy.query(
        "ALTER TABLE shopownedratios ADD PRIMARY KEY (shop_nr, obj_nr)"));

      // shopownedauction and shopownedloans: index only (no clear natural key)
      assert(
        sneezy.query("ALTER TABLE shopownedauction "
                     "MODIFY shop_nr int NOT NULL, "
                     "ADD INDEX idx_shopownedauction_shop_nr (shop_nr)"));
      assert(
        sneezy.query("ALTER TABLE shopownedloans "
                     "MODIFY shop_nr int NOT NULL, "
                     "ADD INDEX idx_shopownedloans_shop_nr (shop_nr)"));

      // FK cascades: all shopowned children -> shopowned (two-level cascade)
      assert(
        sneezy.query("ALTER TABLE shopownedaccess ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedauction ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedbank ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedcorpbank ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedloanrate ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedloans ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedmatch ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedplayer ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedratios ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
      assert(
        sneezy.query("ALTER TABLE shopownedrepair ADD FOREIGN KEY (shop_nr) "
                     "REFERENCES shopowned (shop_nr) ON DELETE CASCADE"));
    },
    [&]() {
      vlogf(LOG_MISC, "Creating player_affect table");
      assert(sneezy.query(
        "CREATE TABLE IF NOT EXISTS player_affect ("
        "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "player_id BIGINT UNSIGNED NOT null, "
        "type SMALLINT NOT null, "
        "level TINYINT NOT null, "
        "duration INT NOT null, "
        "renew INT NOT null, "
        "modifier BIGINT NOT null, "
        "modifier2 BIGINT NOT null, "
        "location TINYINT UNSIGNED NOT null, "
        "bitvector BIGINT UNSIGNED NOT null, "
        "CONSTRAINT fk_player_affect_player "
        "FOREIGN KEY (player_id) REFERENCES player(id) ON DELETE CASCADE)"));
    },
    // Migration 11: Drop unused tables (cleanup for existing databases).
    // New instances never create these tables (seed files removed), but
    // databases predating this branch still have them.
    [&]() {
      vlogf(LOG_MISC, "Dropping unused tables");

      for (const auto* table :
        {"usagelogs", "usagelogsarchive", "objlog", "cgisession", "pings",
          "shoplogarchive", "trophymob", "itemtypes", "material"}) {
        assert(sneezy.query("DROP TABLE IF EXISTS %s", table));
      }

      // Dead views: no code references, broken DEFINER prevents execution
      for (const auto* view : {"qts", "shop_overview"}) {
        assert(sneezy.query("DROP VIEW IF EXISTS %s", view));
      }
    },
  };

  int oldVersion = getVersion(sneezy);
  int newVersion = static_cast<int>(migrations.size());

  vlogf(LOG_MISC,
    boost::format("Running migrations %d -> %d") % oldVersion % newVersion);
  for (int i = oldVersion; i < newVersion; ++i) {
    migrations.at(i)();
    // Bump version after each migration so a crash mid-batch doesn't
    // re-run already-completed migrations.
    assert(sneezy.query(
      "update configuration set value = '%i' where config = 'version'", i + 1));
  }
  vlogf(LOG_MISC, "Migrations done");
}
