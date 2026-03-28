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

  bool hasPrimaryKey(TDatabase& db, const char* table) {
    db.query(
      "SELECT COUNT(*) AS cnt FROM information_schema.TABLE_CONSTRAINTS "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' "
      "AND CONSTRAINT_TYPE='PRIMARY KEY'",
      table);
    return db.fetchRow() && convertTo<int>(db["cnt"]) > 0;
  }

  bool hasIndex(TDatabase& db, const char* table, const char* indexName) {
    db.query(
      "SELECT COUNT(*) AS cnt FROM information_schema.STATISTICS "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' "
      "AND INDEX_NAME='%s'",
      table, indexName);
    return db.fetchRow() && convertTo<int>(db["cnt"]) > 0;
  }

  bool columnIsType(TDatabase& db, const char* table, const char* column,
    const char* expectedType, bool notNull = false) {
    db.query(
      "SELECT COLUMN_TYPE AS col_type, IS_NULLABLE AS nullable "
      "FROM information_schema.COLUMNS "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' "
      "AND COLUMN_NAME='%s'",
      table, column);
    if (!db.fetchRow() || sstring(db["col_type"]) != expectedType) {
      return false;
    }
    // Verify nullability matches the requirement in both directions
    return notNull ? sstring(db["nullable"]) == "NO"
                   : sstring(db["nullable"]) == "YES";
  }

  void modifyColumnType(TDatabase& db, const char* table, const char* column,
    const char* type, bool notNull) {
    if (!columnIsType(db, table, column, type, notNull)) {
      if (notNull) {
        assert(db.query("ALTER TABLE %s MODIFY %s %s NOT null", table, column,
          type));
      } else {
        assert(db.query("ALTER TABLE %s MODIFY %s %s", table, column, type));
      }
    }
  }
  bool hasForeignKey(TDatabase& db, const char* table, const char* column,
    const char* refTable, const char* refColumn = nullptr,
    const char* onDelete = nullptr) {
    // Uses std::format instead of db.query() %s placeholders because the
    // query is built incrementally from optional clauses.
    auto query = std::format(
      "SELECT COUNT(*) AS cnt "
      "FROM information_schema.KEY_COLUMN_USAGE k "
      "JOIN information_schema.REFERENTIAL_CONSTRAINTS r "
      "ON k.CONSTRAINT_SCHEMA = r.CONSTRAINT_SCHEMA "
      "AND k.CONSTRAINT_NAME = r.CONSTRAINT_NAME "
      "WHERE k.TABLE_SCHEMA=DATABASE() AND k.TABLE_NAME='{}' "
      "AND k.COLUMN_NAME='{}' AND k.REFERENCED_TABLE_NAME='{}'",
      table, column, refTable);
    if (refColumn) {
      query += std::format(" AND k.REFERENCED_COLUMN_NAME='{}'", refColumn);
    }
    if (onDelete) {
      query += std::format(" AND r.DELETE_RULE='{}'", onDelete);
    }
    db.query(query.c_str());
    return db.fetchRow() && convertTo<int>(db["cnt"]) > 0;
  }

  bool hasColumn(TDatabase& db, const char* table, const char* column) {
    db.query(
      "SELECT 1 FROM information_schema.COLUMNS "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' AND COLUMN_NAME='%s'",
      table, column);
    return db.fetchRow();
  }

  void addForeignKey(TDatabase& db, const char* table, const char* column,
    const char* refTable, const char* refColumn, const char* onDelete) {
    if (!hasForeignKey(db, table, column, refTable, refColumn, onDelete)) {
      // Explicit constraint name (fk_table_column) ensures database-wide
      // uniqueness, which MariaDB 10.x requires for InnoDB FKs.  MariaDB 12
      // relaxed this to per-table, but auto-generates short names like "1"
      // that produce non-portable dumps.
      assert(
        db.query("ALTER TABLE %s ADD CONSTRAINT fk_%s_%s "
                 "FOREIGN KEY (%s) REFERENCES %s (%s) ON DELETE %s",
          table, table, column, column, refTable, refColumn, onDelete));
    }
  }

  void addCrossDbForeignKey(TDatabase& db, const char* table,
    const char* column, const char* refSchema, const char* refTable,
    const char* refColumn, const char* onDelete) {
    db.query(
      "SELECT COUNT(*) AS cnt "
      "FROM information_schema.KEY_COLUMN_USAGE k "
      "JOIN information_schema.REFERENTIAL_CONSTRAINTS r "
      "ON k.CONSTRAINT_SCHEMA = r.CONSTRAINT_SCHEMA "
      "AND k.CONSTRAINT_NAME = r.CONSTRAINT_NAME "
      "WHERE k.TABLE_SCHEMA=DATABASE() AND k.TABLE_NAME='%s' "
      "AND k.COLUMN_NAME='%s' AND k.REFERENCED_TABLE_SCHEMA='%s' "
      "AND k.REFERENCED_TABLE_NAME='%s' "
      "AND k.REFERENCED_COLUMN_NAME='%s' AND r.DELETE_RULE='%s'",
      table, column, refSchema, refTable, refColumn, onDelete);
    if (db.fetchRow() && convertTo<int>(db["cnt"]) > 0)
      return;
    assert(db.query(
      "ALTER TABLE %s ADD CONSTRAINT fk_%s_%s "
      "FOREIGN KEY (%s) REFERENCES %s.%s (%s) ON DELETE %s",
      table, table, column, column, refSchema, refTable, refColumn, onDelete));
  }

  void addCompositeForeignKey(TDatabase& db, const char* table,
    const char* col1, const char* col2, const char* refTable,
    const char* refCol1, const char* refCol2, const char* onDelete) {
    db.query(
      "SELECT COUNT(*) AS cnt FROM information_schema.TABLE_CONSTRAINTS "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' "
      "AND CONSTRAINT_NAME='fk_%s_%s_%s' AND CONSTRAINT_TYPE='FOREIGN KEY'",
      table, table, col1, col2);
    if (db.fetchRow() && convertTo<int>(db["cnt"]) > 0)
      return;
    assert(
      db.query("ALTER TABLE %s ADD CONSTRAINT fk_%s_%s_%s "
               "FOREIGN KEY (%s, %s) REFERENCES %s (%s, %s) ON DELETE %s",
        table, table, col1, col2, col1, col2, refTable, refCol1, refCol2,
        onDelete));
  }

  void dropForeignKeyIfExists(TDatabase& db, const char* table,
    const char* column, const char* refTable) {
    if (!hasForeignKey(db, table, column, refTable))
      return;
    db.query(
      "SELECT CONSTRAINT_NAME AS fk_name "
      "FROM information_schema.KEY_COLUMN_USAGE "
      "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='%s' "
      "AND COLUMN_NAME='%s' AND REFERENCED_TABLE_NAME='%s'",
      table, column, refTable);
    if (db.fetchRow()) {
      sstring name = db["fk_name"];
      assert(
        db.query("ALTER TABLE %s DROP FOREIGN KEY %s", table, name.c_str()));
    }
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
      if (!hasIndex(sneezy, "shop", "idx_shop_keeper"))
        assert(
          sneezy.query("ALTER TABLE shop ADD INDEX idx_shop_keeper (keeper)"));
      if (!hasIndex(sneezy, "shop", "idx_shop_in_room"))
        assert(sneezy.query(
          "ALTER TABLE shop ADD INDEX idx_shop_in_room (in_room)"));
      if (!hasIndex(sneezy, "shopowned", "idx_shopowned_corp_id"))
        assert(sneezy.query(
          "ALTER TABLE shopowned ADD INDEX idx_shopowned_corp_id (corp_id)"));
      if (!hasIndex(sneezy, "shopowned", "idx_shopowned_tax_nr"))
        assert(sneezy.query(
          "ALTER TABLE shopowned ADD INDEX idx_shopowned_tax_nr (tax_nr)"));

      // Primary keys on direct shop children
      // ALTER IGNORE discards duplicate rows during PK creation.
      // ALGORITHM=COPY is required for IGNORE to work on InnoDB in
      // MariaDB 10.x; INPLACE silently disregards IGNORE there.
      if (!hasPrimaryKey(sneezy, "shoptype"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shoptype "
                       "ADD PRIMARY KEY (shop_nr, type), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopproducing"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopproducing "
                       "ADD PRIMARY KEY (shop_nr, producing), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopmaterial"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopmaterial "
                       "ADD PRIMARY KEY (shop_nr, mat_type), ALGORITHM=COPY"));

      // FK cascades: direct shop children -> shop
      addForeignKey(sneezy, "shoptype", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopproducing", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopmaterial", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopowned", "shop_nr", "shop", "shop_nr",
        "CASCADE");

      // FK: shopowned.corp_id -> corporation (SET NULL on corp delete)
      // corp_id must match corporation.corp_id's type (bigint unsigned)
      assert(sneezy.query(
        "ALTER TABLE shopowned MODIFY corp_id bigint(20) unsigned null"));
      addForeignKey(sneezy, "shopowned", "corp_id", "corporation", "corp_id",
        "SET null");

      // Fix nullable columns on shopowned children, then add PKs.
      // ALGORITHM=COPY: see comment above re MariaDB 10.x IGNORE handling.
      if (!hasPrimaryKey(sneezy, "shopownedbank"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedbank "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY player_id int NOT null, "
                       "ADD PRIMARY KEY (shop_nr, player_id), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedcorpbank"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedcorpbank "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY corp_id int NOT null, "
                       "ADD PRIMARY KEY (shop_nr, corp_id), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedloanrate"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedloanrate "
                       "MODIFY shop_nr int NOT null, "
                       "ADD PRIMARY KEY (shop_nr), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedmatch"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedmatch "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY match_str varchar(255) NOT null, "
                       "ADD PRIMARY KEY (shop_nr, match_str), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedplayer"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedplayer "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY player varchar(80) NOT null, "
                       "ADD PRIMARY KEY (shop_nr, player), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedrepair"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedrepair "
                       "MODIFY shop_nr int NOT null, "
                       "ADD PRIMARY KEY (shop_nr), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedaccess"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedaccess "
                       "ADD PRIMARY KEY (shop_nr, name), ALGORITHM=COPY"));
      if (!hasPrimaryKey(sneezy, "shopownedratios"))
        assert(
          sneezy.query("ALTER IGNORE TABLE shopownedratios "
                       "ADD PRIMARY KEY (shop_nr, obj_nr), ALGORITHM=COPY"));

      // shopownedauction and shopownedloans: index only (no clear natural key)
      if (!hasIndex(sneezy, "shopownedauction", "idx_shopownedauction_shop_nr"))
        assert(
          sneezy.query("ALTER TABLE shopownedauction "
                       "MODIFY shop_nr int NOT null, "
                       "ADD INDEX idx_shopownedauction_shop_nr (shop_nr)"));
      if (!hasIndex(sneezy, "shopownedloans", "idx_shopownedloans_shop_nr"))
        assert(
          sneezy.query("ALTER TABLE shopownedloans "
                       "MODIFY shop_nr int NOT null, "
                       "ADD INDEX idx_shopownedloans_shop_nr (shop_nr)"));

      // FK cascades: all shopowned children -> shopowned (two-level cascade)
      addForeignKey(sneezy, "shopownedaccess", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedauction", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedbank", "shop_nr", "shopowned", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopownedcorpbank", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedloanrate", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedloans", "shop_nr", "shopowned", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopownedmatch", "shop_nr", "shopowned", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shopownedplayer", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedratios", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shopownedrepair", "shop_nr", "shopowned",
        "shop_nr", "CASCADE");
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
    // Drop unused tables (cleanup for existing databases).
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
    // Convert UNIQUE KEY to PRIMARY KEY on identity tables
    [&]() {
      vlogf(LOG_MISC, "Fixing primary keys on identity tables");

      if (!hasPrimaryKey(sneezy, "account")) {
        assert(
          sneezy.query("ALTER TABLE account ADD PRIMARY KEY (account_id), "
                       "DROP KEY account_id"));
      }

      if (!hasPrimaryKey(sneezy, "player")) {
        assert(
          sneezy.query("ALTER TABLE player ADD PRIMARY KEY (id), DROP KEY id"));
      }

      if (!hasPrimaryKey(sneezy, "corporation")) {
        assert(
          sneezy.query("ALTER TABLE corporation ADD PRIMARY KEY (corp_id), "
                       "DROP KEY corp_id"));
      }

      if (!hasPrimaryKey(sneezy, "mail")) {
        assert(sneezy.query(
          "ALTER TABLE mail ADD PRIMARY KEY (mailid), DROP KEY mailid"));
      }

      if (!hasPrimaryKey(sneezy, "immortal_exchange_coin")) {
        assert(
          sneezy.query("ALTER TABLE immortal_exchange_coin "
                       "DROP KEY ix__immortal_exchange_coin__1, "
                       "ADD PRIMARY KEY (k_coin)"));
      }
    },
    // Add critical missing indexes, drop redundant ones
    [&]() {
      vlogf(LOG_MISC, "Adding missing indexes and dropping redundant ones");

      // rent(owner_type, owner) - every item load does a full table scan
      // without this
      if (!hasIndex(sneezy, "rent", "idx_rent_owner")) {
        assert(sneezy.query(
          "ALTER TABLE rent ADD INDEX idx_rent_owner (owner_type, owner)"));
      }

      // objaffect(vnum) - 15K rows, queried by vnum, no index
      if (!hasIndex(sneezy, "objaffect", "idx_objaffect_vnum")) {
        assert(sneezy.query(
          "ALTER TABLE objaffect ADD INDEX idx_objaffect_vnum (vnum)"));
      }

      // objextra(vnum) - 6.4K rows, queried by vnum, no index
      if (!hasIndex(sneezy, "objextra", "idx_objextra_vnum")) {
        assert(sneezy.query(
          "ALTER TABLE objextra ADD INDEX idx_objextra_vnum (vnum)"));
      }

      // corplog(corp_id) - 3.6K rows, queried by corp_id, no index
      if (!hasIndex(sneezy, "corplog", "idx_corplog_corp_id")) {
        assert(sneezy.query(
          "ALTER TABLE corplog ADD INDEX idx_corplog_corp_id (corp_id)"));
      }

      // mail(mailto) - has_mail queries WHERE mailto=? with no index
      if (!hasIndex(sneezy, "mail", "idx_mail_mailto")) {
        assert(
          sneezy.query("ALTER TABLE mail ADD INDEX idx_mail_mailto (mailto)"));
      }

      // Drop duplicate index on configuration (both 'config' UNIQUE and
      // 'idx_configuration_key' UNIQUE cover the same column)
      if (hasIndex(sneezy, "configuration", "idx_configuration_key")) {
        assert(sneezy.query(
          "ALTER TABLE configuration DROP KEY idx_configuration_key"));
      }

      // Drop ix_player_name_account_id - it's fully covered by
      // player_unq_name (UNIQUE on name) for name lookups, and account_id
      // lookups go through other paths. Keep player_unq_name to preserve
      // global name uniqueness (critical for a MUD where players are
      // addressed by name).
      if (hasIndex(sneezy, "player", "ix_player_name_account_id")) {
        assert(sneezy.query(
          "ALTER TABLE player DROP KEY ix_player_name_account_id"));
      }
    },
    // Fix log table timestamps
    [&]() {
      vlogf(LOG_MISC, "Fixing log timestamps");

      // Remove ON UPDATE current_timestamp() from audit/log tables - updates
      // should not silently overwrite the original timestamp
      assert(sneezy.query(
        "ALTER TABLE shoplog "
        "MODIFY logtime timestamp NOT null DEFAULT current_timestamp()"));
      assert(sneezy.query(
        "ALTER TABLE tellhistory "
        "MODIFY telltime timestamp NOT null DEFAULT current_timestamp()"));
      assert(sneezy.query(
        "ALTER TABLE corplog "
        "MODIFY logtime timestamp NOT null DEFAULT current_timestamp()"));
    },
    // Fix FK type mismatches (int -> bigint unsigned)
    [&]() {
      vlogf(LOG_MISC, "Fixing FK column type mismatches");

      constexpr auto bigintU = "bigint(20) unsigned";

      // player.account_id: int -> bigint unsigned (to match account.account_id)
      modifyColumnType(sneezy, "player", "account_id", bigintU, false);
      modifyColumnType(sneezy, "ship_master", "account_id", bigintU, false);

      // player_id columns: int -> bigint unsigned (to match player.id)
      for (auto [table, notNull] :
        std::initializer_list<std::pair<const char*, bool>>{
          {"blockedlist", false},
          {"corpaccess", false},
          {"drug_use", false},
          {"gamblers", true},
          {"pet", false},
          {"playerprompt", false},
          {"ship_master", false},
          {"shopownedbank", true},
          {"shopownedloans", false},
          {"trophy", true},
          {"trophyplayer", true},
          {"wizpower", false},
        }) {
        modifyColumnType(sneezy, table, "player_id", bigintU, notNull);
      }

      // corp_id columns: int -> bigint unsigned (to match corporation.corp_id)
      for (auto [table, notNull] :
        std::initializer_list<std::pair<const char*, bool>>{
          {"corpaccess", true},
          {"corplog", false},
          {"shopownedcorpbank", true},
        }) {
        modifyColumnType(sneezy, table, "corp_id", bigintU, notNull);
      }
    },
    // Add primary keys to tables with clean natural keys
    [&]() {
      vlogf(LOG_MISC, "Adding primary keys to tables with natural keys");

      // Tables with case-insensitive collation collisions (permadeath) or
      // legitimately different data sharing the same natural key (tattoos)
      // are excluded - they need manual data cleanup before PKs can be added.

      // Combined DROP KEY + ADD PRIMARY KEY in single ALTER statements so
      // either both succeed or neither does - no partial state where the
      // old index is gone but the PK failed to add.
      // Also MODIFY NULLable PK columns to NOT NULL in the same ALTER.
      if (!hasPrimaryKey(sneezy, "trophy")) {
        assert(
          sneezy.query("ALTER TABLE trophy "
                       "DROP KEY trophy_idx, "
                       "ADD PRIMARY KEY (player_id, mobvnum)"));
      }
      if (!hasPrimaryKey(sneezy, "trophyplayer")) {
        assert(
          sneezy.query("ALTER TABLE trophyplayer "
                       "DROP KEY ix_trophyplayer_player_id, "
                       "ADD PRIMARY KEY (player_id)"));
      }
      if (!hasPrimaryKey(sneezy, "wizpower")) {
        assert(
          sneezy.query("ALTER TABLE wizpower "
                       "DROP KEY wizpower_idx, "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "MODIFY wizpower int NOT null, "
                       "ADD PRIMARY KEY (player_id, wizpower)"));
      }
      if (!hasPrimaryKey(sneezy, "drug_use")) {
        assert(
          sneezy.query("ALTER TABLE drug_use "
                       "DROP KEY ix_drug_use_player_id, "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "MODIFY drug_id int NOT null, "
                       "ADD PRIMARY KEY (player_id, drug_id)"));
      }
      if (!hasPrimaryKey(sneezy, "fishkeeper")) {
        assert(
          sneezy.query("ALTER TABLE fishkeeper "
                       "DROP KEY ix_fishkeeper_name, "
                       "MODIFY name varchar(80) NOT null, "
                       "ADD PRIMARY KEY (name)"));
      }

      // Tables without redundant indexes to drop
      if (!hasPrimaryKey(sneezy, "globaltoggles")) {
        assert(
          sneezy.query("ALTER TABLE globaltoggles "
                       "MODIFY tog_id int NOT null, "
                       "ADD PRIMARY KEY (tog_id)"));
      }
      if (!hasPrimaryKey(sneezy, "gamblers")) {
        assert(
          sneezy.query("ALTER TABLE gamblers ADD PRIMARY KEY (player_id)"));
      }
      if (!hasPrimaryKey(sneezy, "factionmembers")) {
        assert(
          sneezy.query("ALTER TABLE factionmembers "
                       "MODIFY name varchar(80) NOT null, "
                       "ADD PRIMARY KEY (name)"));
      }
    },
    // Seed shoplogaccountchart and rename shopownednpcloan
    [&]() {
      vlogf(LOG_MISC, "Seeding shoplogaccountchart and fixing table names");

      // The shoplogaccountchart table is the chart of accounts for double-entry
      // bookkeeping in shopaccounting.cc. It maps post_ref integers to account
      // names used by TShopJournal. Without this data, all JOINs to this table
      // return zero rows and financial statements are blank.
      sneezy.query("SELECT 1 FROM shoplogaccountchart LIMIT 1");
      if (!sneezy.fetchRow()) {
        assert(sneezy.query(
          "INSERT INTO shoplogaccountchart (post_ref, name) VALUES "
          "(100, 'Cash'), "
          "(101, 'Dividends'), "
          "(130, 'Inventory'), "
          "(300, 'Paid-in Capital'), "
          "(310, 'Deposits'), "
          "(500, 'Sales'), "
          "(510, 'Recycling'), "
          "(600, 'COGS'), "
          "(610, 'Interest'), "
          "(630, 'Expenses'), "
          "(700, 'Tax'), "
          "(800, 'Retained Earnings')"));
      }

      // Code in spec_mobs_loan_manager.cc queries "shopownednpcloans" (plural)
      // but the table was created as "shopownednpcloan" (singular). The table
      // is empty and has never worked. Rename to match the code and sibling
      // table "shopownedloans".
      sneezy.query(
        "SELECT 1 FROM information_schema.TABLES "
        "WHERE TABLE_SCHEMA=DATABASE() "
        "AND TABLE_NAME='shopownednpcloan'");
      if (sneezy.fetchRow()) {
        assert(
          sneezy.query("RENAME TABLE shopownednpcloan TO shopownednpcloans"));
      }
    },
    // Convert mail.timesent from varchar(32) to TIMESTAMP
    [&]() {
      vlogf(LOG_MISC, "Converting mail.timesent to TIMESTAMP");

      if (!columnIsType(sneezy, "mail", "timesent", "timestamp")) {
        // Convert existing asctime strings (e.g. "Tue May 18 18:22:18 2021")
        // to TIMESTAMP via an intermediate column. STR_TO_DATE with the
        // asctime format handles the conversion.
        assert(
          sneezy.query("ALTER TABLE mail ADD COLUMN IF NOT EXISTS timesent_new "
                       "TIMESTAMP null"));
        sneezy.query(
          "UPDATE mail SET timesent_new = "
          "STR_TO_DATE(timesent, '%%a %%b %%e %%T %%Y') "
          "WHERE timesent IS NOT null");
        assert(
          sneezy.query("ALTER TABLE mail DROP COLUMN timesent, "
                       "CHANGE COLUMN timesent_new timesent TIMESTAMP null "
                       "AFTER mailto"));
      }
    },
    // Widen passwd to varchar(255)
    [&]() {
      vlogf(LOG_MISC, "Widening passwd column");

      // passwd is varchar(13) sized for DES crypt(3). Any modern hash
      // algorithm (bcrypt=60, argon2=97) won't fit.
      if (!columnIsType(sneezy, "account", "passwd", "varchar(255)"))
        assert(sneezy.query(
          "ALTER TABLE account MODIFY passwd varchar(255) DEFAULT null"));
    },
    // Clean orphaned rows before FK constraints
    [&]() {
      vlogf(LOG_MISC, "Cleaning orphaned rows for FK constraint preparation");

      // Player orphans (CASCADE FKs) - delete rows referencing deleted players.
      // The IS NOT null guard prevents deleting rows with intentionally null
      // player_id (e.g. shopownedloans uses null to mean "no borrower").
      for (const auto* table : {"playerprompt", "trophyplayer", "wizpower",
             "shopownedbank", "trophy", "drug_use", "gamblers", "blockedlist",
             "pet", "shopownedloans", "corpaccess"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN player p ON t.player_id = p.id "
          "WHERE t.player_id IS NOT null AND p.id IS null",
          table);
      }

      // Player orphans (SET null FKs) - clear the reference, keep the row
      sneezy.query(
        "UPDATE ship_master SET player_id = null "
        "WHERE player_id IS NOT null "
        "AND player_id NOT IN (SELECT id FROM player)");
      sneezy.query(
        "UPDATE property SET owner = null "
        "WHERE owner IS NOT null "
        "AND owner NOT IN (SELECT id FROM player)");

      // Account orphans - set to null rather than delete, since the players
      // themselves may still be valid
      sneezy.query(
        "UPDATE player SET account_id = null "
        "WHERE account_id IS NOT null "
        "AND account_id NOT IN (SELECT account_id FROM account)");
      sneezy.query(
        "UPDATE ship_master SET account_id = null "
        "WHERE account_id IS NOT null "
        "AND account_id NOT IN (SELECT account_id FROM account)");

      // Corporation orphans
      for (const auto* table : {"corpaccess", "corplog", "shopownedcorpbank"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN corporation c ON t.corp_id = c.corp_id "
          "WHERE t.corp_id IS NOT null AND c.corp_id IS null",
          table);
      }

      // Shop orphans
      for (const auto* table : {"shoplog", "shoplogcogs", "shoplogjournal",
             "shoplogjournalarchive", "factoryproducing", "factorysupplies"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN shop s ON t.shop_nr = s.shop_nr "
          "WHERE t.shop_nr IS NOT null AND s.shop_nr IS null",
          table);
      }

      // Rent orphans - the rent_obj_aff delete is large (~443K rows) but
      // runs against an indexed column.
      sneezy.query(
        "DELETE t FROM rent_obj_aff t "
        "LEFT JOIN rent r ON t.rent_id = r.rent_id "
        "WHERE t.rent_id IS NOT null AND r.rent_id IS null");
      sneezy.query(
        "DELETE t FROM rent_strung t "
        "LEFT JOIN rent r ON t.rent_id = r.rent_id "
        "WHERE t.rent_id IS NOT null AND r.rent_id IS null");
      // Mail rent_id: convert sentinel 0 and stale references to null
      sneezy.query(
        "UPDATE mail SET rent_id = null "
        "WHERE rent_id IS NOT null "
        "AND rent_id NOT IN (SELECT rent_id FROM rent)");
      assert(sneezy.query("ALTER TABLE mail MODIFY rent_id int DEFAULT null"));

      // Obj orphans
      for (const auto* table : {"objaffect", "objextra"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN obj o ON t.vnum = o.vnum "
          "WHERE t.vnum IS NOT null AND o.vnum IS null",
          table);
      }

      // Mob orphans
      for (const auto* table : {"mobresponses", "mob_extra", "mob_imm"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN mob m ON t.vnum = m.vnum "
          "WHERE t.vnum IS NOT null AND m.vnum IS null",
          table);
      }

      // Room orphans
      for (const auto* table : {"roomexit", "roomextra"}) {
        sneezy.query(
          "DELETE t FROM %s t "
          "LEFT JOIN room r ON t.vnum = r.vnum "
          "WHERE t.vnum IS NOT null AND r.vnum IS null",
          table);
      }

      // Fix property.owner type to match player.id
      modifyColumnType(sneezy, "property", "owner", "bigint(20) unsigned",
        false);
    },
    // Add foreign key constraints
    [&]() {
      vlogf(LOG_MISC, "Adding foreign key constraints");

      // player.id children - CASCADE (deletion should clean up all player
      // data, fixing the incomplete cleanup in both self-delete and nuke paths)
      addForeignKey(sneezy, "trophy", "player_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "trophyplayer", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "playerprompt", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "wizpower", "player_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "drug_use", "player_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "gamblers", "player_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "blockedlist", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "pet", "player_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "shopownedbank", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "shopownedloans", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "corpaccess", "player_id", "player", "id",
        "CASCADE");
      addForeignKey(sneezy, "ship_master", "player_id", "player", "id",
        "SET null");

      // account.account_id children - CASCADE for player (account deletion
      // should delete all characters), SET null for ships (persist unclaimed)
      addForeignKey(sneezy, "player", "account_id", "account", "account_id",
        "CASCADE");
      addForeignKey(sneezy, "ship_master", "account_id", "account",
        "account_id", "SET null");

      // corporation.corp_id children
      addForeignKey(sneezy, "corpaccess", "corp_id", "corporation", "corp_id",
        "CASCADE");
      addForeignKey(sneezy, "corplog", "corp_id", "corporation", "corp_id",
        "CASCADE");
      addForeignKey(sneezy, "shopownedcorpbank", "corp_id", "corporation",
        "corp_id", "CASCADE");

      // shop.shop_nr children
      addForeignKey(sneezy, "shoplog", "shop_nr", "shop", "shop_nr", "CASCADE");
      addForeignKey(sneezy, "shoplogcogs", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shoplogjournal", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "shoplogjournalarchive", "shop_nr", "shop",
        "shop_nr", "CASCADE");
      addForeignKey(sneezy, "factoryproducing", "shop_nr", "shop", "shop_nr",
        "CASCADE");
      addForeignKey(sneezy, "factorysupplies", "shop_nr", "shop", "shop_nr",
        "CASCADE");

      // rent.rent_id children
      addForeignKey(sneezy, "rent_obj_aff", "rent_id", "rent", "rent_id",
        "CASCADE");
      addForeignKey(sneezy, "rent_strung", "rent_id", "rent", "rent_id",
        "CASCADE");
      addForeignKey(sneezy, "mail", "rent_id", "rent", "rent_id", "SET null");

      // obj.vnum children
      addForeignKey(sneezy, "objaffect", "vnum", "obj", "vnum", "CASCADE");
      addForeignKey(sneezy, "objextra", "vnum", "obj", "vnum", "CASCADE");

      // mob.vnum children
      addForeignKey(sneezy, "mobresponses", "vnum", "mob", "vnum", "CASCADE");
      addForeignKey(sneezy, "mob_extra", "vnum", "mob", "vnum", "CASCADE");
      addForeignKey(sneezy, "mob_imm", "vnum", "mob", "vnum", "CASCADE");

      // room.vnum children
      addForeignKey(sneezy, "roomexit", "vnum", "room", "vnum", "CASCADE");
      addForeignKey(sneezy, "roomextra", "vnum", "room", "vnum", "CASCADE");

      // property.owner -> player.id (property persists without owner)
      addForeignKey(sneezy, "property", "owner", "player", "id", "SET null");
    },
    // Natural PKs on clean tables (~26 tables)
    [&]() {
      vlogf(LOG_MISC, "Adding natural primary keys to clean tables");

      // property: UNIQUE KEY id -> PRIMARY KEY
      if (!hasPrimaryKey(sneezy, "property")) {
        assert(sneezy.query(
          "ALTER TABLE property ADD PRIMARY KEY (id), DROP KEY id"));
      }

      // playerprompt: 1:1 with player
      if (!hasPrimaryKey(sneezy, "playerprompt")) {
        assert(
          sneezy.query("ALTER TABLE playerprompt "
                       "DROP KEY ix_playerprompt_player_id, "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "ADD PRIMARY KEY (player_id)"));
      }

      // fishlargest: one record per fish vnum
      if (!hasPrimaryKey(sneezy, "fishlargest")) {
        assert(
          sneezy.query("ALTER TABLE fishlargest "
                       "DROP KEY ix_fishlargest_vnum, "
                       "MODIFY vnum int NOT null, "
                       "ADD PRIMARY KEY (vnum)"));
      }

      if (!hasPrimaryKey(sneezy, "brickquest")) {
        assert(sneezy.query("ALTER TABLE brickquest ADD PRIMARY KEY (name)"));
      }

      if (!hasPrimaryKey(sneezy, "poll")) {
        assert(sneezy.query("ALTER TABLE poll ADD PRIMARY KEY (poll_id)"));
      }

      if (!hasPrimaryKey(sneezy, "poll_option")) {
        assert(sneezy.query(
          "ALTER TABLE poll_option ADD PRIMARY KEY (poll_id, option_id)"));
      }

      if (!hasPrimaryKey(sneezy, "poll_vote")) {
        assert(sneezy.query(
          "ALTER TABLE poll_vote ADD PRIMARY KEY (account, poll_id)"));
      }

      if (!hasPrimaryKey(sneezy, "lowtasks")) {
        assert(
          sneezy.query("ALTER TABLE lowtasks "
                       "MODIFY id int NOT null, "
                       "ADD PRIMARY KEY (id)"));
      }

      if (!hasPrimaryKey(sneezy, "quest_limbs_team")) {
        assert(sneezy.query(
          "ALTER TABLE quest_limbs_team ADD PRIMARY KEY (player)"));
      }

      // ship_destinations: drop redundant index, MODIFY NOT null
      if (!hasPrimaryKey(sneezy, "ship_destinations")) {
        assert(
          sneezy.query("ALTER TABLE ship_destinations "
                       "DROP KEY ix1__ship_destinations, "
                       "MODIFY vnum int NOT null, "
                       "MODIFY name varchar(32) NOT null, "
                       "ADD PRIMARY KEY (vnum, name)"));
      }

      // Factory tables
      if (!hasPrimaryKey(sneezy, "factoryblueprint")) {
        assert(
          sneezy.query("ALTER TABLE factoryblueprint "
                       "MODIFY vnum int NOT null, "
                       "MODIFY supplytype int NOT null, "
                       "ADD PRIMARY KEY (vnum, supplytype)"));
      }

      if (!hasPrimaryKey(sneezy, "factoryproducing")) {
        assert(
          sneezy.query("ALTER TABLE factoryproducing "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY vnum int NOT null, "
                       "ADD PRIMARY KEY (shop_nr, vnum)"));
      }

      if (!hasPrimaryKey(sneezy, "factorysupplies")) {
        assert(
          sneezy.query("ALTER TABLE factorysupplies "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY supplytype int NOT null, "
                       "ADD PRIMARY KEY (shop_nr, supplytype)"));
      }

      if (!hasPrimaryKey(sneezy, "shoplogaccountchart")) {
        assert(
          sneezy.query("ALTER TABLE shoplogaccountchart "
                       "MODIFY post_ref int NOT null, "
                       "ADD PRIMARY KEY (post_ref)"));
      }

      if (!hasPrimaryKey(sneezy, "shopownedcentralbank")) {
        assert(
          sneezy.query("ALTER TABLE shopownedcentralbank "
                       "MODIFY bank int NOT null, "
                       "ADD PRIMARY KEY (bank)"));
      }

      // shopownedloans: player_id type already fixed by migration 14.
      // Delete any rows with null player_id (orphaned loans with no borrower)
      // before adding the NOT null constraint and PK.
      if (!hasPrimaryKey(sneezy, "shopownedloans")) {
        sneezy.query("DELETE FROM shopownedloans WHERE player_id IS null");
        assert(
          sneezy.query("ALTER TABLE shopownedloans "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "ADD PRIMARY KEY (shop_nr, player_id)"));
      }

      // PK (shop_nr, player_id) makes the single-column shop_nr index redundant
      if (hasIndex(sneezy, "shopownedloans", "idx_shopownedloans_shop_nr"))
        assert(
          sneezy.query("ALTER TABLE shopownedloans "
                       "DROP INDEX idx_shopownedloans_shop_nr"));

      if (!hasPrimaryKey(sneezy, "pet")) {
        assert(
          sneezy.query("ALTER TABLE pet "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "MODIFY vnum int NOT null, "
                       "ADD PRIMARY KEY (player_id, vnum)"));
      }

      if (!hasPrimaryKey(sneezy, "blockedlist")) {
        assert(
          sneezy.query("ALTER TABLE blockedlist "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "MODIFY blocked varchar(33) NOT null, "
                       "ADD PRIMARY KEY (player_id, blocked)"));
      }

      // corpaccess: PK + unique index on (corp_id, name) for common query
      if (!hasPrimaryKey(sneezy, "corpaccess")) {
        assert(
          sneezy.query("ALTER TABLE corpaccess "
                       "MODIFY player_id bigint(20) unsigned NOT null, "
                       "ADD PRIMARY KEY (corp_id, player_id)"));
      }
      if (!hasIndex(sneezy, "corpaccess", "idx_corpaccess_corp_name")) {
        assert(sneezy.query(
          "ALTER TABLE corpaccess "
          "ADD UNIQUE INDEX idx_corpaccess_corp_name (corp_id, name)"));
      }

      if (!hasPrimaryKey(sneezy, "mobresponses")) {
        assert(sneezy.query("ALTER TABLE mobresponses ADD PRIMARY KEY (vnum)"));
      }
      // mobresponses_idx(vnum) is now redundant with PRIMARY KEY(vnum)
      if (hasIndex(sneezy, "mobresponses", "mobresponses_idx"))
        assert(
          sneezy.query("ALTER TABLE mobresponses DROP INDEX mobresponses_idx"));

      // shopownedauction: ticket is the natural key
      if (!hasPrimaryKey(sneezy, "shopownedauction")) {
        assert(
          sneezy.query("ALTER TABLE shopownedauction "
                       "MODIFY ticket int NOT null, "
                       "ADD PRIMARY KEY (ticket)"));
      }

      // shopownednpcloans: renamed from shopownednpcloan in migration 16
      if (!hasPrimaryKey(sneezy, "shopownednpcloans")) {
        assert(
          sneezy.query("ALTER TABLE shopownednpcloans "
                       "MODIFY loan_id int NOT null, "
                       "ADD PRIMARY KEY (loan_id)"));
      }
    },
    // Natural PKs with dedup
    [&]() {
      vlogf(LOG_MISC, "Deduplicating and adding natural primary keys");

      // permadeath: 1 case-insensitive collision
      if (!hasPrimaryKey(sneezy, "permadeath")) {
        sneezy.query(
          "DELETE p1 FROM permadeath p1 "
          "INNER JOIN permadeath p2 "
          "ON p1.name = p2.name "
          "AND BINARY p1.name != BINARY p2.name "
          "AND p1.level < p2.level");
        assert(sneezy.query("ALTER TABLE permadeath ADD PRIMARY KEY (name)"));
      }

      // tattoos: 2 pairs of dupes
      if (!hasPrimaryKey(sneezy, "tattoos")) {
        // Drop stale temp table from a previous partial run before RENAME
        assert(sneezy.query("DROP TABLE IF EXISTS tattoos_old"));
        assert(
          sneezy.query("CREATE TABLE IF NOT EXISTS tattoos_clean "
                       "LIKE tattoos"));
        sneezy.query("TRUNCATE TABLE tattoos_clean");
        assert(
          sneezy.query("INSERT INTO tattoos_clean "
                       "SELECT name, MIN(tattoo), location "
                       "FROM tattoos GROUP BY name, location"));
        assert(sneezy.query(
          "RENAME TABLE tattoos TO tattoos_old, tattoos_clean TO tattoos"));
        assert(sneezy.query("DROP TABLE IF EXISTS tattoos_old"));
        assert(
          sneezy.query("ALTER TABLE tattoos ADD PRIMARY KEY (name, location)"));
      }

      // rent_strung: orphans already cleaned by migration 19, just add PK
      if (!hasPrimaryKey(sneezy, "rent_strung")) {
        assert(
          sneezy.query("ALTER TABLE rent_strung "
                       "DROP KEY rent_strung_idx, "
                       "ADD PRIMARY KEY (rent_id)"));
      }

      // The following dedup blocks use DELETE + INSERT inside a transaction
      // instead of TRUNCATE. TRUNCATE is DDL (auto-commits), so a crash
      // between TRUNCATE and INSERT would lose data with no rollback.
      // DELETE is DML and participates in the transaction, so a crash
      // rolls back to the original data.

      // objextra: 1 duplicate row
      if (!hasPrimaryKey(sneezy, "objextra")) {
        assert(
          sneezy.query("CREATE TABLE IF NOT EXISTS objextra_dedup AS "
                       "SELECT vnum, name, MIN(description) as description "
                       "FROM objextra GROUP BY vnum, name"));
        sneezy.query("BEGIN");
        assert(sneezy.query("DELETE FROM objextra"));
        assert(
          sneezy.query("INSERT INTO objextra SELECT * FROM objextra_dedup"));
        sneezy.query("COMMIT");
        assert(sneezy.query("DROP TABLE IF EXISTS objextra_dedup"));
        assert(
          sneezy.query("ALTER TABLE objextra ADD PRIMARY KEY (vnum, name)"));
      }

      // PK (vnum, name) makes the single-column vnum index redundant
      if (hasIndex(sneezy, "objextra", "idx_objextra_vnum"))
        assert(
          sneezy.query("ALTER TABLE objextra DROP INDEX idx_objextra_vnum"));

      // shoplogcogs: ~3590 excess rows, aggregate duplicates
      if (!hasPrimaryKey(sneezy, "shoplogcogs")) {
        assert(
          sneezy.query("CREATE TABLE IF NOT EXISTS shoplogcogs_dedup AS "
                       "SELECT shop_nr, obj_name, "
                       "SUM(count) as count, SUM(total_cost) as total_cost "
                       "FROM shoplogcogs GROUP BY shop_nr, obj_name"));
        sneezy.query("BEGIN");
        assert(sneezy.query("DELETE FROM shoplogcogs"));
        assert(sneezy.query(
          "INSERT INTO shoplogcogs SELECT * FROM shoplogcogs_dedup"));
        sneezy.query("COMMIT");
        assert(sneezy.query("DROP TABLE IF EXISTS shoplogcogs_dedup"));
        assert(
          sneezy.query("ALTER TABLE shoplogcogs "
                       "DROP KEY shoplogcogs_idx, "
                       "MODIFY shop_nr int NOT null, "
                       "MODIFY obj_name varchar(128) NOT null, "
                       "ADD PRIMARY KEY (shop_nr, obj_name)"));
      }

      // roomexit: ~14485 excess rows, prefer door rows
      if (!hasPrimaryKey(sneezy, "roomexit")) {
        assert(sneezy.query(
          "CREATE TABLE IF NOT EXISTS roomexit_clean AS "
          "SELECT vnum, direction, name, description, type, "
          "condition_flag, lock_difficulty, weight, key_num, destination "
          "FROM ("
          "  SELECT *, ROW_NUMBER() OVER ("
          "    PARTITION BY vnum, direction "
          "    ORDER BY type DESC, condition_flag DESC, "
          "    lock_difficulty DESC"
          "  ) as rn "
          "  FROM roomexit"
          ") ranked WHERE rn = 1"));
        sneezy.query("BEGIN");
        assert(sneezy.query("DELETE FROM roomexit"));
        assert(
          sneezy.query("INSERT INTO roomexit SELECT * FROM roomexit_clean"));
        sneezy.query("COMMIT");
        assert(sneezy.query("DROP TABLE IF EXISTS roomexit_clean"));
        assert(
          sneezy.query("ALTER TABLE roomexit "
                       "DROP KEY roomexit_idx, "
                       "ADD PRIMARY KEY (vnum, direction)"));
      }

      // roomextra: ~274 excess rows, TEXT->VARCHAR(255) for name
      if (!hasPrimaryKey(sneezy, "roomextra")) {
        assert(sneezy.query(
          "CREATE TABLE IF NOT EXISTS roomextra_dedup AS "
          "SELECT vnum, CAST(name AS CHAR(255)) as name, "
          "MIN(description) as description "
          "FROM roomextra GROUP BY vnum, CAST(name AS CHAR(255))"));
        assert(sneezy.query(
          "ALTER TABLE roomextra MODIFY name VARCHAR(255) NOT null"));
        sneezy.query("BEGIN");
        assert(sneezy.query("DELETE FROM roomextra"));
        assert(
          sneezy.query("INSERT INTO roomextra SELECT * FROM roomextra_dedup"));
        sneezy.query("COMMIT");
        assert(sneezy.query("DROP TABLE IF EXISTS roomextra_dedup"));
        assert(
          sneezy.query("ALTER TABLE roomextra "
                       "DROP KEY roomextra_idx, "
                       "ADD PRIMARY KEY (vnum, name)"));
      }
    },
    // Surrogate PKs on small/empty tables
    [&]() {
      vlogf(LOG_MISC, "Adding surrogate primary keys to small tables");

      for (const auto* table :
        {"board_message", "corplog", "querytimes", "quest_limbs", "wholist"}) {
        if (!hasPrimaryKey(sneezy, table)) {
          if (!hasColumn(sneezy, table, "id"))
            assert(sneezy.query(
              "ALTER TABLE %s "
              "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
              "PRIMARY KEY FIRST",
              table));
        }
      }

      // ship_master: surrogate PK (player_id can be null due to SET null FK)
      if (!hasPrimaryKey(sneezy, "ship_master")) {
        if (!hasColumn(sneezy, "ship_master", "id"))
          assert(sneezy.query(
            "ALTER TABLE ship_master "
            "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
            "PRIMARY KEY FIRST"));
      }
    },
    // Surrogate PKs on large tables
    [&]() {
      vlogf(LOG_MISC, "Adding surrogate primary keys to large tables");

      for (const auto* table : {"rent_obj_aff", "shoplog", "tellhistory"}) {
        if (!hasPrimaryKey(sneezy, table)) {
          if (!hasColumn(sneezy, table, "id"))
            assert(sneezy.query(
              "ALTER TABLE %s "
              "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
              "PRIMARY KEY FIRST",
              table));
        }
      }

      // shoplogjournal: journal_id is AUTO_INCREMENT, so we must remove that
      // attribute before dropping its key (MariaDB requires AUTO_INCREMENT
      // columns to always be part of a key). Do it all in one ALTER.
      if (!hasPrimaryKey(sneezy, "shoplogjournal")) {
        if (!hasColumn(sneezy, "shoplogjournal", "id"))
          assert(sneezy.query(
            "ALTER TABLE shoplogjournal "
            "MODIFY journal_id int NOT null, "
            "DROP KEY journal_id, "
            "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
            "PRIMARY KEY FIRST"));
      }

      // shoplogjournalarchive: same pattern
      if (!hasPrimaryKey(sneezy, "shoplogjournalarchive")) {
        if (!hasColumn(sneezy, "shoplogjournalarchive", "id"))
          assert(sneezy.query(
            "ALTER TABLE shoplogjournalarchive "
            "MODIFY journal_id int NOT null, "
            "DROP KEY journal_id, "
            "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
            "PRIMARY KEY FIRST"));
      }
    },
    // objaffect dedup + surrogate PK
    [&]() {
      vlogf(LOG_MISC, "Deduplicating objaffect and adding surrogate PK");

      if (!hasPrimaryKey(sneezy, "objaffect")) {
        assert(sneezy.query(
          "CREATE TABLE IF NOT EXISTS objaffect_dedup AS "
          "SELECT DISTINCT vnum, type, mod1, mod2 FROM objaffect"));
        sneezy.query("BEGIN");
        assert(sneezy.query("DELETE FROM objaffect"));
        assert(
          sneezy.query("INSERT INTO objaffect SELECT * FROM objaffect_dedup"));
        sneezy.query("COMMIT");
        assert(sneezy.query("DROP TABLE IF EXISTS objaffect_dedup"));
        if (!hasColumn(sneezy, "objaffect", "id"))
          assert(sneezy.query(
            "ALTER TABLE objaffect "
            "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
            "PRIMARY KEY FIRST"));
      }
    },
    // Immortal database PKs
    [&]() {
      vlogf(LOG_MISC, "Adding primary keys to immortal database tables");

      // mobresponses: natural PK on (owner, vnum)
      if (!hasPrimaryKey(immortal, "mobresponses")) {
        assert(immortal.query(
          "UPDATE mobresponses SET owner = '' WHERE owner IS null"));
        assert(
          immortal.query("ALTER TABLE mobresponses "
                         "MODIFY owner VARCHAR(32) NOT null DEFAULT '', "
                         "ADD PRIMARY KEY (owner, vnum)"));
      }

      // Surrogate PKs on tables with heavy duplication from builder saves
      for (const auto* table :
        {"objaffect", "objextra", "roomexit", "roomextra"}) {
        if (!hasPrimaryKey(immortal, table)) {
          if (!hasColumn(immortal, table, "id"))
            assert(immortal.query(
              "ALTER TABLE %s "
              "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
              "PRIMARY KEY FIRST",
              table));
        }
      }
    },
    // Convert all tables to utf8mb4
    [&]() {
      vlogf(LOG_MISC, "Converting all tables to utf8mb4");

      auto convertDatabase = [](TDatabase& db, const char* schema) {
        // Collect table names first - can't interleave queries on the
        // shared connection
        std::vector<sstring> tables;
        db.query(
          "SELECT TABLE_NAME FROM information_schema.TABLES "
          "WHERE TABLE_SCHEMA='%s' AND TABLE_TYPE='BASE TABLE' "
          "AND TABLE_COLLATION <> 'utf8mb4_general_ci'",
          schema);
        while (db.fetchRow())
          tables.push_back(db[0u]);

        if (tables.empty())
          return;

        // Disable FK checks - parent/child tables will temporarily have
        // mismatched charsets between individual ALTERs
        assert(db.query("SET FOREIGN_KEY_CHECKS=0"));

        for (const auto& table : tables)
          assert(
            db.query("ALTER TABLE %s CONVERT TO CHARACTER SET utf8mb4 "
                     "COLLATE utf8mb4_general_ci",
              table.c_str()));

        assert(db.query("SET FOREIGN_KEY_CHECKS=1"));

        // Update database default for new tables
        assert(
          db.query("ALTER DATABASE %s CHARACTER SET utf8mb4 "
                   "COLLATE utf8mb4_general_ci",
            schema));
      };

      convertDatabase(sneezy, "sneezy");
      convertDatabase(immortal, "immortal");
    },
    // Drop corpaccess.name (queries now use player_id)
    [&]() {
      vlogf(LOG_MISC, "Dropping corpaccess.name column");

      if (hasIndex(sneezy, "corpaccess", "idx_corpaccess_corp_name"))
        assert(sneezy.query(
          "ALTER TABLE corpaccess DROP INDEX idx_corpaccess_corp_name"));

      if (hasColumn(sneezy, "corpaccess", "name"))
        assert(sneezy.query("ALTER TABLE corpaccess DROP COLUMN name"));
    },
    // Migrate tattoos from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating tattoos from name to player_id");

      if (!hasColumn(sneezy, "tattoos", "player_id")) {
        assert(
          sneezy.query("ALTER TABLE tattoos "
                       "ADD COLUMN player_id BIGINT UNSIGNED DEFAULT null"));
      }

      // Backfill and cleanup: run whenever legacy column still exists
      // (handles both fresh run and crash-after-ADD-COLUMN retry)
      if (hasColumn(sneezy, "tattoos", "name")) {
        assert(
          sneezy.query("UPDATE tattoos t JOIN player p ON t.name = p.name "
                       "SET t.player_id = p.id"));
        assert(sneezy.query("DELETE FROM tattoos WHERE player_id IS null"));
        assert(
          sneezy.query("ALTER TABLE tattoos "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN name, "
                       "MODIFY player_id BIGINT UNSIGNED NOT null, "
                       "ADD PRIMARY KEY (player_id, location)"));
      }

      addForeignKey(sneezy, "tattoos", "player_id", "player", "id", "CASCADE");
    },
    // Add player_id to permadeath (keep name for display)
    [&]() {
      vlogf(LOG_MISC, "Adding player_id to permadeath");

      if (!hasColumn(sneezy, "permadeath", "player_id")) {
        assert(
          sneezy.query("ALTER TABLE permadeath "
                       "ADD COLUMN player_id BIGINT UNSIGNED DEFAULT null"));
      }

      // Backfill outside hasColumn guard so it's crash-resumable -
      // orphans kept with null player_id (historical memorial records)
      assert(
        sneezy.query("UPDATE permadeath pd JOIN player p ON pd.name = p.name "
                     "SET pd.player_id = p.id "
                     "WHERE pd.player_id IS null"));

      // Swap PK from (name) to surrogate id - name stays for display,
      // player_id may be null for deleted players
      if (!hasColumn(sneezy, "permadeath", "id")) {
        assert(
          sneezy.query("ALTER TABLE permadeath "
                       "DROP PRIMARY KEY, "
                       "ADD COLUMN id BIGINT UNSIGNED NOT null AUTO_INCREMENT "
                       "PRIMARY KEY FIRST"));
      }

      addForeignKey(sneezy, "permadeath", "player_id", "player", "id",
        "SET null");
    },
    // Migrate fishlargest + fishkeeper to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating fishlargest and fishkeeper to player_id");

      // fishlargest: add player_id, keep name for historical display
      if (!hasColumn(sneezy, "fishlargest", "player_id")) {
        assert(
          sneezy.query("ALTER TABLE fishlargest "
                       "ADD COLUMN player_id BIGINT UNSIGNED DEFAULT null"));
      }

      // Backfill outside hasColumn guard so it's crash-resumable -
      // rows with 'no one' or deleted players keep null player_id
      assert(
        sneezy.query("UPDATE fishlargest fl JOIN player p ON fl.name = p.name "
                     "SET fl.player_id = p.id "
                     "WHERE fl.player_id IS null"));

      addForeignKey(sneezy, "fishlargest", "player_id", "player", "id",
        "SET null");

      // fishkeeper: add player_id, drop name
      if (!hasColumn(sneezy, "fishkeeper", "player_id")) {
        assert(
          sneezy.query("ALTER TABLE fishkeeper "
                       "ADD COLUMN player_id BIGINT UNSIGNED DEFAULT null"));
      }

      if (hasColumn(sneezy, "fishkeeper", "name")) {
        assert(
          sneezy.query("UPDATE fishkeeper fk JOIN player p ON fk.name = p.name "
                       "SET fk.player_id = p.id"));
        assert(sneezy.query("DELETE FROM fishkeeper WHERE player_id IS null"));
        // Swap PK from (name) to (player_id)
        assert(
          sneezy.query("ALTER TABLE fishkeeper "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN name, "
                       "MODIFY player_id BIGINT UNSIGNED NOT null, "
                       "ADD PRIMARY KEY (player_id)"));
      }

      addForeignKey(sneezy, "fishkeeper", "player_id", "player", "id",
        "CASCADE");
    },
    // Migrate factionmembers from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating factionmembers from name to player_id");

      // Table is rebuilt from scratch on every rollcall (TRUNCATE + INSERT),
      // so just restructure the schema. Next rollcall will populate with
      // player_ids.
      if (hasColumn(sneezy, "factionmembers", "name")) {
        assert(sneezy.query("TRUNCATE TABLE factionmembers"));
        assert(
          sneezy.query("ALTER TABLE factionmembers "
                       "DROP COLUMN name, "
                       "ADD COLUMN player_id BIGINT UNSIGNED NOT null FIRST, "
                       "ADD PRIMARY KEY (player_id)"));
      }

      addForeignKey(sneezy, "factionmembers", "player_id", "player", "id",
        "CASCADE");

      if (!hasIndex(sneezy, "factionmembers", "idx_factionmembers_faction")) {
        assert(
          sneezy.query("ALTER TABLE factionmembers "
                       "ADD INDEX idx_factionmembers_faction (faction)"));
      }
    },
    // Migrate tellhistory from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating tellhistory to player_id");

      if (hasColumn(sneezy, "tellhistory", "tellfrom")) {
        // Step 1: Delete orphan rows where neither sender nor recipient
        // matches an existing player
        assert(
          sneezy.query("DELETE th FROM tellhistory th "
                       "LEFT JOIN player p1 ON th.tellfrom=p1.name "
                       "LEFT JOIN player p2 ON th.tellto=p2.name "
                       "WHERE p1.id IS null AND p2.id IS null"));

        // Step 2: Keep only the most recent 25 per recipient.
        // Use the existing `id` surrogate PK (from migration 24) to identify
        // rows for deletion.
        assert(
          sneezy.query("DELETE FROM tellhistory WHERE id NOT IN ("
                       "SELECT id FROM ("
                       "SELECT id, ROW_NUMBER() OVER "
                       "(PARTITION BY tellto ORDER BY telltime DESC) AS rn "
                       "FROM tellhistory) ranked WHERE rn <= 25)"));

        // Step 3: Add from_id/to_id and backfill
        if (!hasColumn(sneezy, "tellhistory", "from_id"))
          assert(
            sneezy.query("ALTER TABLE tellhistory "
                         "ADD COLUMN from_id BIGINT UNSIGNED DEFAULT null, "
                         "ADD COLUMN to_id BIGINT UNSIGNED DEFAULT null"));

        assert(
          sneezy.query("UPDATE tellhistory th "
                       "JOIN player p ON th.tellfrom=p.name "
                       "SET th.from_id=p.id"));
        assert(
          sneezy.query("UPDATE tellhistory th "
                       "JOIN player p ON th.tellto=p.name "
                       "SET th.to_id=p.id"));

        // Step 4: Drop old name columns
        assert(
          sneezy.query("ALTER TABLE tellhistory "
                       "DROP COLUMN tellfrom, "
                       "DROP COLUMN tellto"));

        // Drop old indexes that referenced tellfrom/tellto
        if (hasIndex(sneezy, "tellhistory", "tellhistory_idx"))
          assert(
            sneezy.query("ALTER TABLE tellhistory DROP INDEX tellhistory_idx"));
        if (hasIndex(sneezy, "tellhistory", "tellfrom"))
          assert(sneezy.query("ALTER TABLE tellhistory DROP INDEX tellfrom"));
      }

      // Add index for the read query pattern (outside hasColumn guard so it
      // survives a crash between DROP COLUMN and this point)
      if (!hasIndex(sneezy, "tellhistory", "idx_tellhistory_to"))
        assert(
          sneezy.query("ALTER TABLE tellhistory "
                       "ADD INDEX idx_tellhistory_to (to_id, telltime)"));

      addForeignKey(sneezy, "tellhistory", "to_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "tellhistory", "from_id", "player", "id",
        "SET null");
    },
    // Migrate mail from name columns to player_id FKs
    [&]() {
      vlogf(LOG_MISC, "Migrating mail to player_id foreign keys");

      // Add from_id and to_id columns
      if (!hasColumn(sneezy, "mail", "to_id")) {
        assert(
          sneezy.query("ALTER TABLE mail "
                       "ADD COLUMN to_id BIGINT UNSIGNED DEFAULT null, "
                       "ADD COLUMN from_id BIGINT UNSIGNED DEFAULT null"));
      }

      // Backfill and finalize (guarded by old column - re-runs safely if
      // server crashed after ADD COLUMN but before completion)
      if (hasColumn(sneezy, "mail", "mailto")) {
        // Backfill to_id from mailto
        assert(
          sneezy.query("UPDATE mail m JOIN player p ON m.mailto=p.name "
                       "SET m.to_id=p.id"));
        // Backfill from_id from mailfrom (only for player senders).
        // System/NPC senders with no player match keep from_id as null.
        assert(
          sneezy.query("UPDATE mail m JOIN player p ON m.mailfrom=p.name "
                       "SET m.from_id=p.id"));

        // Delete orphan mail where recipient no longer exists
        assert(sneezy.query("DELETE FROM mail WHERE to_id IS null"));

        // Drop old index on mailto (created by migration 12)
        if (hasIndex(sneezy, "mail", "idx_mail_mailto"))
          assert(sneezy.query("ALTER TABLE mail DROP INDEX idx_mail_mailto"));

        assert(
          sneezy.query("ALTER TABLE mail "
                       "DROP COLUMN mailto, "
                       "MODIFY to_id BIGINT UNSIGNED NOT null"));
      }

      // Add index for the read query pattern (has_mail, read_delete).
      // Independent guard: if server crashed after DROP COLUMN mailto but
      // before this, the hasColumn guard above won't re-enter.
      if (!hasIndex(sneezy, "mail", "idx_mail_to")) {
        assert(
          sneezy.query("ALTER TABLE mail "
                       "ADD INDEX idx_mail_to (to_id, port)"));
      }

      addForeignKey(sneezy, "mail", "to_id", "player", "id", "CASCADE");
      addForeignKey(sneezy, "mail", "from_id", "player", "id", "SET null");
    },
    // Migrate shopownedaccess from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating shopownedaccess to player_id");

      if (!hasColumn(sneezy, "shopownedaccess", "player_id")) {
        assert(
          sneezy.query("ALTER TABLE shopownedaccess "
                       "ADD COLUMN player_id BIGINT UNSIGNED DEFAULT null"));
      }

      if (hasColumn(sneezy, "shopownedaccess", "name")) {
        assert(
          sneezy.query("UPDATE shopownedaccess soa "
                       "JOIN player p ON soa.name = p.name "
                       "SET soa.player_id = p.id"));
        assert(
          sneezy.query("DELETE FROM shopownedaccess WHERE player_id IS null"));
        // Swap PK from (shop_nr, name) to (shop_nr, player_id)
        assert(
          sneezy.query("ALTER TABLE shopownedaccess "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN name, "
                       "MODIFY player_id BIGINT UNSIGNED NOT null, "
                       "ADD PRIMARY KEY (shop_nr, player_id)"));
      }

      addForeignKey(sneezy, "shopownedaccess", "player_id", "player", "id",
        "CASCADE");
    },
    // Rename numeric FK constraint names to fk_table_column.
    // MariaDB 12 auto-generates short numeric names ("1", "2") that are
    // only per-table unique.  MariaDB 10 requires per-database uniqueness,
    // making dumps from 12 non-restorable on 10.
    [&]() {
      vlogf(LOG_MISC,
        "Renaming numeric FK names for cross-version compatibility");

      struct FKInfo {
          sstring table, constraint, column, refTable, refColumn, deleteRule;
      };
      std::vector<FKInfo> fks;

      sneezy.query(
        "SELECT kcu.TABLE_NAME AS tbl, kcu.CONSTRAINT_NAME AS fk_name, "
        "kcu.COLUMN_NAME AS col, "
        "kcu.REFERENCED_TABLE_NAME AS ref_tbl, "
        "kcu.REFERENCED_COLUMN_NAME AS ref_col, "
        "rc.DELETE_RULE AS del_rule "
        "FROM information_schema.KEY_COLUMN_USAGE kcu "
        "JOIN information_schema.REFERENTIAL_CONSTRAINTS rc "
        "ON rc.CONSTRAINT_SCHEMA = kcu.TABLE_SCHEMA "
        "AND rc.CONSTRAINT_NAME = kcu.CONSTRAINT_NAME "
        "AND rc.TABLE_NAME = kcu.TABLE_NAME "
        "WHERE kcu.TABLE_SCHEMA = DATABASE() "
        "AND kcu.REFERENCED_TABLE_NAME IS NOT null "
        "AND kcu.CONSTRAINT_NAME REGEXP '^[0-9]+$'");

      while (sneezy.fetchRow()) {
        fks.push_back({sneezy["tbl"], sneezy["fk_name"], sneezy["col"],
          sneezy["ref_tbl"], sneezy["ref_col"], sneezy["del_rule"]});
      }

      for (const auto& fk : fks) {
        auto newName = "fk_" + fk.table + "_" + fk.column;
        assert(
          sneezy.query("ALTER TABLE %s DROP FOREIGN KEY `%s`, "
                       "ADD CONSTRAINT %s FOREIGN KEY (%s) REFERENCES %s (%s) "
                       "ON DELETE %s",
            fk.table.c_str(), fk.constraint.c_str(), newName.c_str(),
            fk.column.c_str(), fk.refTable.c_str(), fk.refColumn.c_str(),
            fk.deleteRule.c_str()));
      }
    },
    // Add missing FK constraints
    [&]() {
      vlogf(LOG_MISC, "Adding missing FK constraints");

      // shopownedcentralbank.bank -> shop.shop_nr
      sneezy.query(
        "DELETE FROM shopownedcentralbank "
        "WHERE bank NOT IN (SELECT shop_nr FROM shop)");
      addForeignKey(sneezy, "shopownedcentralbank", "bank", "shop", "shop_nr",
        "CASCADE");

      // shopownedcentralbank.centralbank -> shop.shop_nr
      sneezy.query(
        "DELETE FROM shopownedcentralbank "
        "WHERE centralbank IS NOT null "
        "AND centralbank NOT IN (SELECT shop_nr FROM shop)");
      addForeignKey(sneezy, "shopownedcentralbank", "centralbank", "shop",
        "shop_nr", "CASCADE");

      // shopownednpcloans.owner -> player.id (owner stores player_id per code)
      modifyColumnType(sneezy, "shopownednpcloans", "owner",
        "bigint(20) unsigned", false);
      sneezy.query(
        "UPDATE shopownednpcloans SET owner=null "
        "WHERE owner IS NOT null AND owner NOT IN (SELECT id FROM player)");
      addForeignKey(sneezy, "shopownednpcloans", "owner", "player", "id",
        "SET null");

      // shopownedauction.shop_nr already references shopowned.shop_nr (from
      // migration 20), which cascades through shopowned -> shop. No direct FK
      // to shop needed.

      // shopownedauction.seller -> player.id (widen first)
      modifyColumnType(sneezy, "shopownedauction", "seller",
        "bigint(20) unsigned", false);
      sneezy.query(
        "UPDATE shopownedauction SET seller=null "
        "WHERE seller IS NOT null AND seller NOT IN (SELECT id FROM player)");
      addForeignKey(sneezy, "shopownedauction", "seller", "player", "id",
        "SET null");

      // shopownedauction.bidder -> player.id (widen first)
      modifyColumnType(sneezy, "shopownedauction", "bidder",
        "bigint(20) unsigned", false);
      sneezy.query(
        "UPDATE shopownedauction SET bidder=null "
        "WHERE bidder IS NOT null AND bidder NOT IN (SELECT id FROM player)");
      addForeignKey(sneezy, "shopownedauction", "bidder", "player", "id",
        "SET null");

      // shoplogjournal.post_ref -> shoplogaccountchart.post_ref
      sneezy.query(
        "DELETE FROM shoplogjournal "
        "WHERE post_ref IS NOT null "
        "AND post_ref NOT IN (SELECT post_ref FROM shoplogaccountchart)");
      addForeignKey(sneezy, "shoplogjournal", "post_ref", "shoplogaccountchart",
        "post_ref", "CASCADE");

      // shoplogjournalarchive.post_ref -> shoplogaccountchart.post_ref
      // (same schema as shoplogjournal, populated by archiving)
      sneezy.query(
        "DELETE a FROM shoplogjournalarchive a "
        "LEFT JOIN shoplogaccountchart c ON a.post_ref = c.post_ref "
        "WHERE a.post_ref IS NOT null AND c.post_ref IS null");
      addForeignKey(sneezy, "shoplogjournalarchive", "post_ref",
        "shoplogaccountchart", "post_ref", "CASCADE");

      // poll_option.poll_id -> poll.poll_id
      sneezy.query(
        "DELETE FROM poll_option "
        "WHERE poll_id NOT IN (SELECT poll_id FROM poll)");
      addForeignKey(sneezy, "poll_option", "poll_id", "poll", "poll_id",
        "CASCADE");

      // poll_vote.poll_id -> poll.poll_id
      sneezy.query(
        "DELETE FROM poll_vote "
        "WHERE poll_id NOT IN (SELECT poll_id FROM poll)");
      addForeignKey(sneezy, "poll_vote", "poll_id", "poll", "poll_id",
        "CASCADE");

      // FKs referencing world-data tables (mob.vnum, obj.vnum, room.vnum)
      // are added in the world-data FK migration later in this file.

      // shopowned.tax_nr -> shopowned.shop_nr (self-referential)
      addForeignKey(sneezy, "shopowned", "tax_nr", "shopowned", "shop_nr",
        "SET null");

      // corporation.bank -> shop.shop_nr
      sneezy.query(
        "UPDATE corporation SET bank=null "
        "WHERE bank IS NOT null "
        "AND bank NOT IN (SELECT shop_nr FROM shop)");
      addForeignKey(sneezy, "corporation", "bank", "shop", "shop_nr",
        "SET null");
    },
    // Migrate shopownedplayer from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating shopownedplayer from name to player_id");

      if (hasColumn(sneezy, "shopownedplayer", "player")) {
        if (!hasColumn(sneezy, "shopownedplayer", "player_id"))
          assert(sneezy.query(
            "ALTER TABLE shopownedplayer "
            "ADD COLUMN player_id BIGINT(20) UNSIGNED AFTER shop_nr"));

        assert(
          sneezy.query("UPDATE shopownedplayer sop "
                       "JOIN player p ON p.name=sop.player "
                       "SET sop.player_id=p.id"));

        // Drop orphans (player no longer exists)
        assert(
          sneezy.query("DELETE FROM shopownedplayer WHERE player_id IS null"));

        assert(
          sneezy.query("ALTER TABLE shopownedplayer "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN player, "
                       "ADD PRIMARY KEY (shop_nr, player_id)"));
      }

      addForeignKey(sneezy, "shopownedplayer", "player_id", "player", "id",
        "CASCADE");
    },
    // Migrate brickquest from name to player_id
    [&]() {
      vlogf(LOG_MISC, "Migrating brickquest from name to player_id");

      if (hasColumn(sneezy, "brickquest", "name")) {
        if (!hasColumn(sneezy, "brickquest", "player_id"))
          assert(
            sneezy.query("ALTER TABLE brickquest "
                         "ADD COLUMN player_id BIGINT(20) UNSIGNED"));

        assert(
          sneezy.query("UPDATE brickquest b "
                       "JOIN player p ON p.name=b.name "
                       "SET b.player_id=p.id"));

        assert(sneezy.query("DELETE FROM brickquest WHERE player_id IS null"));

        assert(
          sneezy.query("ALTER TABLE brickquest "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN name, "
                       "ADD PRIMARY KEY (player_id)"));
      }

      addForeignKey(sneezy, "brickquest", "player_id", "player", "id",
        "CASCADE");
    },
    // Migrate quest_limbs tables from name to player_id
    [&]() {
      vlogf(LOG_MISC,
        "Migrating quest_limbs and quest_limbs_team to player_id");

      // quest_limbs_team: player varchar(80) PK -> player_id bigint unsigned
      if (hasColumn(sneezy, "quest_limbs_team", "player")) {
        if (!hasColumn(sneezy, "quest_limbs_team", "player_id"))
          assert(
            sneezy.query("ALTER TABLE quest_limbs_team "
                         "ADD COLUMN player_id BIGINT(20) UNSIGNED"));

        assert(
          sneezy.query("UPDATE quest_limbs_team qlt "
                       "JOIN player p ON p.name=qlt.player "
                       "SET qlt.player_id=p.id"));

        assert(
          sneezy.query("DELETE FROM quest_limbs_team WHERE player_id IS null"));

        assert(
          sneezy.query("ALTER TABLE quest_limbs_team "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN player, "
                       "ADD PRIMARY KEY (player_id)"));
      }

      addForeignKey(sneezy, "quest_limbs_team", "player_id", "player", "id",
        "CASCADE");

      // quest_limbs: player varchar(80) -> player_id bigint unsigned
      if (hasColumn(sneezy, "quest_limbs", "player")) {
        if (!hasColumn(sneezy, "quest_limbs", "player_id"))
          assert(
            sneezy.query("ALTER TABLE quest_limbs "
                         "ADD COLUMN player_id BIGINT(20) UNSIGNED"));

        assert(
          sneezy.query("UPDATE quest_limbs ql "
                       "JOIN player p ON p.name=ql.player "
                       "SET ql.player_id=p.id"));

        assert(sneezy.query("DELETE FROM quest_limbs WHERE player_id IS null"));

        assert(sneezy.query("ALTER TABLE quest_limbs DROP COLUMN player"));
      }

      addForeignKey(sneezy, "quest_limbs", "player_id", "player", "id",
        "CASCADE");
    },
    // Migrate poll_vote from account name to account_id
    [&]() {
      vlogf(LOG_MISC, "Migrating poll_vote from account name to account_id");

      if (hasColumn(sneezy, "poll_vote", "account")) {
        if (!hasColumn(sneezy, "poll_vote", "account_id"))
          assert(
            sneezy.query("ALTER TABLE poll_vote "
                         "ADD COLUMN account_id BIGINT(20) UNSIGNED"));

        assert(
          sneezy.query("UPDATE poll_vote pv "
                       "JOIN account a ON a.name=pv.account "
                       "SET pv.account_id=a.account_id"));

        assert(sneezy.query("DELETE FROM poll_vote WHERE account_id IS null"));

        assert(
          sneezy.query("ALTER TABLE poll_vote "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN account, "
                       "ADD PRIMARY KEY (account_id, poll_id)"));
      }

      addForeignKey(sneezy, "poll_vote", "account_id", "account", "account_id",
        "CASCADE");
    },
    // Drop duplicate index, add shoplog index, change wholist
    // engine
    [&]() {
      vlogf(LOG_MISC,
        "Dropping duplicate index, adding shoplog index, "
        "changing wholist to MEMORY engine");

      // shoplogjournal has two identical indexes on (shop_nr, sneezy_year)
      if (hasIndex(sneezy, "shoplogjournal", "shoplogjournal_report"))
        assert(sneezy.query(
          "ALTER TABLE shoplogjournal DROP INDEX shoplogjournal_report"));

      // shoplog.name is queried but has no index
      if (!hasIndex(sneezy, "shoplog", "idx_shoplog_shop_name"))
        assert(
          sneezy.query("ALTER TABLE shoplog "
                       "ADD INDEX idx_shoplog_shop_name (shop_nr, name)"));

      // The new composite index makes the old single-column index redundant
      if (hasIndex(sneezy, "shoplog", "shoplog_idx"))
        assert(sneezy.query("ALTER TABLE shoplog DROP INDEX shoplog_idx"));

      // The most common query pattern is WHERE shop_nr=X ORDER BY logtime DESC.
      // The (shop_nr, name) index can't provide logtime ordering, causing a
      // full table scan with filesort. This composite serves both per-shop
      // queries and time-range retention deletes.
      if (!hasIndex(sneezy, "shoplog", "idx_shoplog_shop_logtime"))
        assert(sneezy.query(
          "ALTER TABLE shoplog "
          "ADD INDEX idx_shoplog_shop_logtime (shop_nr, logtime)"));

      // Drop redundant indexes already covered by primary keys
      sneezy.query("DROP INDEX IF EXISTS mobresponses_idx ON mobresponses");
      sneezy.query("DROP INDEX IF EXISTS player_id ON alias");

      // wholist is truncated and rebuilt every 10 seconds; MEMORY engine
      // avoids InnoDB redo log and buffer pool overhead
      assert(sneezy.query("ALTER TABLE wholist ENGINE=MEMORY"));
    },
    // Widen immortal_exchange_coin player ID columns
    [&]() {
      vlogf(LOG_MISC,
        "Widening immortal_exchange_coin columns to match player.id type");

      modifyColumnType(sneezy, "immortal_exchange_coin", "created_by",
        "bigint(20) unsigned", false);
      modifyColumnType(sneezy, "immortal_exchange_coin", "created_for",
        "bigint(20) unsigned", false);
      modifyColumnType(sneezy, "immortal_exchange_coin", "redeemed_by",
        "bigint(20) unsigned", false);
      modifyColumnType(sneezy, "immortal_exchange_coin", "redeemed_for",
        "bigint(20) unsigned", false);
    },
    // Add index on querytimes.date_logged for retention cleanup
    [&]() {
      vlogf(LOG_MISC, "Adding index on querytimes.date_logged");

      if (!hasIndex(sneezy, "querytimes", "idx_querytimes_date"))
        assert(
          sneezy.query("ALTER TABLE querytimes "
                       "ADD INDEX idx_querytimes_date (date_logged)"));
    },
    // Immortal DB cleanup and restructure (owner -> player_id)
    [&]() {
      vlogf(LOG_MISC,
        "Immortal DB cleanup and restructure (owner -> player_id)");

      constexpr const char* allTables[] = {"mob", "mob_imm", "mob_extra",
        "mobresponses", "obj", "objaffect", "objextra", "room", "roomexit",
        "roomextra"};

      // Phase 1: Delete dead data - rows not belonging to active builders.
      // null NOT IN (...) evaluates to null (not true), so handle nulls first.
      for (const auto* t : allTables) {
        immortal.query("DELETE FROM %s WHERE owner IS null", t);
        immortal.query(
          "DELETE FROM %s WHERE owner NOT IN ("
          "SELECT p.name FROM sneezy.player p "
          "JOIN sneezy.wizdata w ON p.id = w.player_id)",
          t);
      }

      // Phase 2: Deduplicate tables with auto-increment id.
      // MariaDB can't DELETE from the same table used in a subquery,
      // so materialize the keeper set into a temp table first.
      if (hasColumn(immortal, "roomextra", "id")) {
        immortal.query(
          "CREATE TEMPORARY TABLE IF NOT EXISTS roomextra_keep AS "
          "SELECT MIN(id) as id FROM roomextra "
          "GROUP BY owner, vnum, name");
        immortal.query(
          "DELETE FROM roomextra "
          "WHERE id NOT IN (SELECT id FROM roomextra_keep)");
        immortal.query("DROP TEMPORARY TABLE IF EXISTS roomextra_keep");
      }

      if (hasColumn(immortal, "roomexit", "id")) {
        immortal.query(
          "CREATE TEMPORARY TABLE IF NOT EXISTS roomexit_keep AS "
          "SELECT MIN(id) as id FROM roomexit "
          "GROUP BY owner, vnum, direction");
        immortal.query(
          "DELETE FROM roomexit "
          "WHERE id NOT IN (SELECT id FROM roomexit_keep)");
        immortal.query("DROP TEMPORARY TABLE IF EXISTS roomexit_keep");
      }

      if (hasColumn(immortal, "objaffect", "id")) {
        immortal.query(
          "CREATE TEMPORARY TABLE IF NOT EXISTS objaffect_keep AS "
          "SELECT MIN(id) as id FROM objaffect "
          "GROUP BY owner, vnum, type, mod1, mod2");
        immortal.query(
          "DELETE FROM objaffect "
          "WHERE id NOT IN (SELECT id FROM objaffect_keep)");
        immortal.query("DROP TEMPORARY TABLE IF EXISTS objaffect_keep");
      }

      if (hasColumn(immortal, "objextra", "id")) {
        immortal.query(
          "CREATE TEMPORARY TABLE IF NOT EXISTS objextra_keep AS "
          "SELECT MIN(id) as id FROM objextra "
          "GROUP BY owner, vnum, name");
        immortal.query(
          "DELETE FROM objextra "
          "WHERE id NOT IN (SELECT id FROM objextra_keep)");
        immortal.query("DROP TEMPORARY TABLE IF EXISTS objextra_keep");
      }

      // Phase 3: Add player_id columns
      for (const auto* t : allTables) {
        if (!hasColumn(immortal, t, "player_id"))
          assert(immortal.query(
            "ALTER TABLE %s ADD COLUMN player_id BIGINT UNSIGNED", t));
      }

      // Phase 4: Populate player_id from owner name
      for (const auto* t : allTables) {
        immortal.query(
          "UPDATE %s SET player_id = ("
          "SELECT id FROM sneezy.player WHERE name = %s.owner"
          ") WHERE player_id IS null OR player_id = 0",
          t, t);
      }

      // Phase 5: Restructure tables - single atomic ALTER per table.
      // Guard: only run if owner column still exists (idempotency).
      if (hasColumn(immortal, "mob", "owner"))
        assert(
          immortal.query("ALTER TABLE mob "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum), "
                         "DROP COLUMN owner, "
                         "MODIFY name varchar(127) NOT null, "
                         "MODIFY short_desc varchar(127) NOT null, "
                         "MODIFY long_desc varchar(255) NOT null"));

      if (hasColumn(immortal, "mob_imm", "owner"))
        assert(
          immortal.query("ALTER TABLE mob_imm "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum, type), "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "mob_extra", "owner"))
        assert(
          immortal.query("ALTER TABLE mob_extra "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum, keyword), "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "mobresponses", "owner"))
        assert(
          immortal.query("ALTER TABLE mobresponses "
                         "DROP PRIMARY KEY, "
                         "DROP INDEX mobresponses_idx, "
                         "ADD PRIMARY KEY (player_id, vnum), "
                         "ADD INDEX mobresponses_idx (vnum), "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "obj", "owner"))
        assert(
          immortal.query("ALTER TABLE obj "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum), "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "objaffect", "owner"))
        assert(
          immortal.query("ALTER TABLE objaffect "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum, type, mod1, mod2), "
                         "DROP COLUMN id, "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "objextra", "owner"))
        assert(
          immortal.query("ALTER TABLE objextra "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum, name), "
                         "DROP COLUMN id, "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "room", "owner"))
        assert(
          immortal.query("ALTER TABLE room "
                         "DROP PRIMARY KEY, "
                         "ADD PRIMARY KEY (player_id, vnum), "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "roomexit", "owner"))
        assert(
          immortal.query("ALTER TABLE roomexit "
                         "DROP PRIMARY KEY, "
                         "DROP INDEX roomexit_idx, "
                         "ADD PRIMARY KEY (player_id, vnum, direction), "
                         "DROP COLUMN id, "
                         "DROP COLUMN owner"));

      if (hasColumn(immortal, "roomextra", "owner"))
        assert(
          immortal.query("ALTER TABLE roomextra "
                         "DROP PRIMARY KEY, "
                         "DROP INDEX roomextra_idx, "
                         "ADD PRIMARY KEY (player_id, vnum, name), "
                         "MODIFY name varchar(255) NOT null, "
                         "DROP COLUMN id, "
                         "DROP COLUMN owner"));

      // Phase 6: Foreign keys
      // Parent FKs (cross-database: immortal -> sneezy)
      addCrossDbForeignKey(immortal, "mob", "player_id", "sneezy", "player",
        "id", "CASCADE");
      addCrossDbForeignKey(immortal, "obj", "player_id", "sneezy", "player",
        "id", "CASCADE");
      addCrossDbForeignKey(immortal, "room", "player_id", "sneezy", "player",
        "id", "CASCADE");

      // Delete orphaned child rows before adding composite FKs.
      // Builders can have e.g. mob responses without a corresponding mob entry.
      immortal.query(
        "DELETE c FROM mob_imm c LEFT JOIN mob p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM mob_extra c LEFT JOIN mob p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM mobresponses c LEFT JOIN mob p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM objaffect c LEFT JOIN obj p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM objextra c LEFT JOIN obj p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM roomexit c LEFT JOIN room p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");
      immortal.query(
        "DELETE c FROM roomextra c LEFT JOIN room p "
        "ON c.player_id=p.player_id AND c.vnum=p.vnum "
        "WHERE p.player_id IS null");

      // Child composite FKs (within immortal)
      addCompositeForeignKey(immortal, "mob_imm", "player_id", "vnum", "mob",
        "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "mob_extra", "player_id", "vnum", "mob",
        "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "mobresponses", "player_id", "vnum",
        "mob", "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "objaffect", "player_id", "vnum", "obj",
        "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "objextra", "player_id", "vnum", "obj",
        "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "roomexit", "player_id", "vnum", "room",
        "player_id", "vnum", "CASCADE");
      addCompositeForeignKey(immortal, "roomextra", "player_id", "vnum", "room",
        "player_id", "vnum", "CASCADE");
    },
    // sneezy.objaffect natural composite PK
    [&]() {
      vlogf(LOG_MISC, "Converting sneezy.objaffect to natural composite PK");

      if (hasColumn(sneezy, "objaffect", "id")) {
        dropForeignKeyIfExists(sneezy, "objaffect", "vnum", "obj");
        if (hasIndex(sneezy, "objaffect", "idx_objaffect_vnum"))
          assert(sneezy.query(
            "ALTER TABLE objaffect DROP INDEX idx_objaffect_vnum"));

        assert(
          sneezy.query("ALTER TABLE objaffect "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id, "
                       "ADD PRIMARY KEY (vnum, type, mod1, mod2)"));

        addForeignKey(sneezy, "objaffect", "vnum", "obj", "vnum", "CASCADE");
      }
    },
    // sneezy.alias dedup + natural composite PK
    [&]() {
      vlogf(LOG_MISC, "Converting sneezy.alias to natural composite PK");

      if (hasColumn(sneezy, "alias", "id")) {
        sneezy.query(
          "CREATE TEMPORARY TABLE alias_keep AS "
          "SELECT MAX(id) as id FROM alias GROUP BY player_id, word");
        sneezy.query(
          "DELETE FROM alias "
          "WHERE id NOT IN (SELECT id FROM alias_keep)");
        sneezy.query("DROP TEMPORARY TABLE alias_keep");

        dropForeignKeyIfExists(sneezy, "alias", "player_id", "player");

        assert(
          sneezy.query("ALTER TABLE alias "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id, "
                       "ADD PRIMARY KEY (player_id, word)"));

        addForeignKey(sneezy, "alias", "player_id", "player", "id", "CASCADE");

        // The FK auto-creates a player_id index, but it's redundant with
        // the PK's leftmost prefix. Drop it.
        if (hasIndex(sneezy, "alias", "player_id"))
          assert(sneezy.query("ALTER TABLE alias DROP INDEX player_id"));
      }
    },
    // Remaining sneezy tables - natural composite PKs
    [&]() {
      vlogf(LOG_MISC,
        "Converting remaining sneezy tables to natural composite PKs");

      // playernotes
      if (hasColumn(sneezy, "playernotes", "id")) {
        dropForeignKeyIfExists(sneezy, "playernotes", "player_id", "player");
        if (hasIndex(sneezy, "playernotes", "player_id"))
          assert(sneezy.query("ALTER TABLE playernotes DROP INDEX player_id"));
        assert(
          sneezy.query("ALTER TABLE playernotes "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id, "
                       "ADD PRIMARY KEY (player_id, name)"));
        addForeignKey(sneezy, "playernotes", "player_id", "player", "id",
          "CASCADE");
      }

      // accountnotes
      if (hasColumn(sneezy, "accountnotes", "id")) {
        dropForeignKeyIfExists(sneezy, "accountnotes", "account_id", "account");
        if (hasIndex(sneezy, "accountnotes", "account_id"))
          assert(
            sneezy.query("ALTER TABLE accountnotes DROP INDEX account_id"));
        assert(
          sneezy.query("ALTER TABLE accountnotes "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id, "
                       "ADD PRIMARY KEY (account_id, name)"));
        addForeignKey(sneezy, "accountnotes", "account_id", "account",
          "account_id", "CASCADE");
      }

      // savedroomsacct
      if (hasColumn(sneezy, "savedroomsacct", "id")) {
        dropForeignKeyIfExists(sneezy, "savedroomsacct", "account_id",
          "account");
        if (hasIndex(sneezy, "savedroomsacct", "account_id"))
          assert(
            sneezy.query("ALTER TABLE savedroomsacct DROP INDEX account_id"));
        assert(
          sneezy.query("ALTER TABLE savedroomsacct "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id, "
                       "ADD PRIMARY KEY (account_id, name)"));
        addForeignKey(sneezy, "savedroomsacct", "account_id", "account",
          "account_id", "CASCADE");
      }

      // configuration
      if (hasColumn(sneezy, "configuration", "id")) {
        assert(
          sneezy.query("ALTER TABLE configuration "
                       "DROP PRIMARY KEY, "
                       "DROP COLUMN id"));
      }

      // Independent guards: if server crashed after DROP COLUMN id but
      // before these, the hasColumn guard above won't re-enter.
      if (!hasPrimaryKey(sneezy, "configuration")) {
        assert(
          sneezy.query("ALTER TABLE configuration ADD PRIMARY KEY (config)"));
      }
      if (hasIndex(sneezy, "configuration", "config"))
        assert(sneezy.query("ALTER TABLE configuration DROP INDEX config"));
      if (hasIndex(sneezy, "configuration", "idx_configuration_key"))
        assert(sneezy.query(
          "ALTER TABLE configuration DROP INDEX idx_configuration_key"));
    },
    // Ensure every corporation has a bank entry. Production data may be
    // missing rows for corps that never deposited - doReserve() silently
    // fails without one, causing shops to accumulate gold indefinitely.
    [&]() {
      vlogf(LOG_MISC, "Inserting missing corp bank entries");
      assert(
        sneezy.query("INSERT IGNORE INTO shopownedcorpbank "
                     "(shop_nr, corp_id, talens, earned_interest) "
                     "SELECT c.bank, c.corp_id, 0, 0 FROM corporation c "
                     "LEFT JOIN shopownedcorpbank cb "
                     "ON c.corp_id = cb.corp_id AND c.bank = cb.shop_nr "
                     "WHERE cb.corp_id IS null"));
    },
    // Add nullability constraints on core columns
    [&]() {
      vlogf(LOG_MISC, "Adding NOT NULL constraints to core columns");

      // Fix any NULLs in numeric columns (should be zero per audit, but
      // ensure ALTER won't fail)
      sneezy.query("UPDATE player SET talens = 0 WHERE talens IS null");
      sneezy.query("UPDATE player SET nutrition = 0 WHERE nutrition IS null");
      sneezy.query("UPDATE account SET email = '' WHERE email IS null");
      sneezy.query("UPDATE rent SET weight = 0 WHERE weight IS null");

      // player.name - UNIQUE constraint makes NULL a logic error
      modifyColumnType(sneezy, "player", "name", "varchar(80)", true);

      // player.talens, player.nutrition
      if (!columnIsType(sneezy, "player", "talens", "int(11)", true))
        assert(sneezy.query(
          "ALTER TABLE player MODIFY talens int(11) NOT NULL DEFAULT 0"));
      if (!columnIsType(sneezy, "player", "nutrition", "int(11)", true))
        assert(sneezy.query(
          "ALTER TABLE player MODIFY nutrition int(11) NOT NULL DEFAULT 0"));

      // account.name, account.passwd - required for login
      modifyColumnType(sneezy, "account", "name", "varchar(80)", true);
      modifyColumnType(sneezy, "account", "passwd", "varchar(255)", true);

      // account.email
      if (!columnIsType(sneezy, "account", "email", "varchar(80)", true))
        assert(sneezy.query(
          "ALTER TABLE account MODIFY email varchar(80) NOT NULL DEFAULT ''"));

      // rent.weight - all other numeric rent columns are already NOT NULL
      if (!columnIsType(sneezy, "rent", "weight", "double", true))
        assert(sneezy.query(
          "ALTER TABLE rent MODIFY weight double NOT NULL DEFAULT 0"));

      // rent.owner_type - polymorphic discriminator must not be nullable.
      // ENUM type string varies by MariaDB version, so check IS_NULLABLE
      // directly instead of using columnIsType.
      sneezy.query(
        "SELECT IS_NULLABLE AS is_nullable "
        "FROM information_schema.COLUMNS "
        "WHERE TABLE_SCHEMA = 'sneezy' AND TABLE_NAME = 'rent' "
        "AND COLUMN_NAME = 'owner_type'");
      if (sneezy.fetchRow() && sstring(sneezy["is_nullable"]) == "YES")
        assert(sneezy.query(
          "ALTER TABLE rent MODIFY "
          "owner_type ENUM('player','shop','room','mail') NOT NULL"));

      // Shop log shop_nr is never null in practice - enforce it
      modifyColumnType(sneezy, "shoplog", "shop_nr", "int(11)", true);
      modifyColumnType(sneezy, "shoplogjournal", "shop_nr", "int(11)", true);
      modifyColumnType(sneezy, "shoplogjournalarchive", "shop_nr", "int(11)",
        true);
    },
    // Right-size text columns
    [&]() {
      vlogf(LOG_MISC, "Converting oversized text columns to appropriate types");

      // shoplogjournal: mediumtext -> VARCHAR
      modifyColumnType(sneezy, "shoplogjournal", "customer_name", "varchar(80)",
        false);
      modifyColumnType(sneezy, "shoplogjournal", "obj_name", "varchar(127)",
        false);

      // shoplogjournalarchive: same changes
      modifyColumnType(sneezy, "shoplogjournalarchive", "customer_name",
        "varchar(80)", false);
      modifyColumnType(sneezy, "shoplogjournalarchive", "obj_name",
        "varchar(127)", false);

      // shoplogaccountchart: mediumtext -> VARCHAR(32)
      modifyColumnType(sneezy, "shoplogaccountchart", "name", "varchar(32)",
        false);

      // board_message: mediumtext -> TEXT (64KB, not 16MB)
      modifyColumnType(sneezy, "board_message", "post", "text", false);
    },
    // Convert char() to varchar() in mob tables
    [&]() {
      vlogf(LOG_MISC,
        "Converting char() to varchar() in sneezy.mob and mob_extra");

      // sneezy.mob
      modifyColumnType(sneezy, "mob", "name", "varchar(127)", true);
      modifyColumnType(sneezy, "mob", "short_desc", "varchar(127)", true);
      modifyColumnType(sneezy, "mob", "long_desc", "varchar(255)", true);
      modifyColumnType(sneezy, "mob", "local_sound", "varchar(255)", false);
      modifyColumnType(sneezy, "mob", "adjacent_sound", "varchar(255)", false);

      // sneezy.mob_extra
      modifyColumnType(sneezy, "mob_extra", "keyword", "varchar(32)", true);
      modifyColumnType(sneezy, "mob_extra", "description", "varchar(255)",
        false);

      // immortal.mob and mob_extra - same columns still use char()
      modifyColumnType(immortal, "mob", "local_sound", "varchar(255)", false);
      modifyColumnType(immortal, "mob", "adjacent_sound", "varchar(255)",
        false);
      modifyColumnType(immortal, "mob_extra", "keyword", "varchar(32)", true);
      modifyColumnType(immortal, "mob_extra", "description", "varchar(255)",
        false);
    },
    // Change zone.zone_enabled to tinyint(1)
    [&]() {
      vlogf(LOG_MISC, "Converting zone.zone_enabled to tinyint(1)");
      sneezy.query(
        "UPDATE zone SET zone_enabled = 0 WHERE zone_enabled IS null");
      if (!columnIsType(sneezy, "zone", "zone_enabled", "tinyint(1)", true))
        assert(
          sneezy.query("ALTER TABLE zone "
                       "MODIFY zone_enabled tinyint(1) NOT NULL DEFAULT 0"));
    },
    // Widen rent_obj_aff columns to match player_affect
    [&]() {
      vlogf(LOG_MISC,
        "Widening rent_obj_aff columns to match player_affect types");

      if (!columnIsType(sneezy, "rent_obj_aff", "modifier", "bigint(20)", true))
        assert(
          sneezy.query("ALTER TABLE rent_obj_aff "
                       "MODIFY modifier bigint(20) NOT NULL DEFAULT 0"));
      if (!columnIsType(sneezy, "rent_obj_aff", "modifier2", "bigint(20)",
            true))
        assert(
          sneezy.query("ALTER TABLE rent_obj_aff "
                       "MODIFY modifier2 bigint(20) NOT NULL DEFAULT 0"));
      if (!columnIsType(sneezy, "rent_obj_aff", "bitvector",
            "bigint(20) unsigned", true))
        assert(sneezy.query(
          "ALTER TABLE rent_obj_aff "
          "MODIFY bitvector bigint(20) unsigned NOT NULL DEFAULT 0"));
    },

    // Add logtime index to shoplogjournalarchive and name snapshots,
    // FKs, indexes to immortal_exchange_coin
    [&]() {
      vlogf(LOG_MISC,
        "Adding logtime index to shoplogjournalarchive and name snapshot "
        "columns, FKs, and indexes to immortal_exchange_coin");

      // Add logtime index for time-based retention queries
      if (!hasIndex(sneezy, "shoplogjournalarchive", "idx_sjarchive_logtime"))
        assert(
          sneezy.query("ALTER TABLE shoplogjournalarchive "
                       "ADD INDEX idx_sjarchive_logtime (logtime)"));

      // Add name snapshot columns to immortal_exchange_coin
      if (!hasColumn(sneezy, "immortal_exchange_coin", "created_by_name"))
        assert(sneezy.query(
          "ALTER TABLE immortal_exchange_coin "
          "ADD COLUMN created_by_name varchar(80) DEFAULT null, "
          "ADD COLUMN created_for_name varchar(80) DEFAULT null, "
          "ADD COLUMN redeemed_by_name varchar(80) DEFAULT null, "
          "ADD COLUMN redeemed_for_name varchar(80) DEFAULT null"));

      // Backfill names from player table.
      // Idempotent: only updates rows where name is still null.
      // Players already deleted get null names (history already lost).
      sneezy.query(
        "UPDATE immortal_exchange_coin SET created_by_name = "
        "(SELECT name FROM player WHERE id = created_by) "
        "WHERE created_by IS NOT null AND created_by_name IS null");
      sneezy.query(
        "UPDATE immortal_exchange_coin SET created_for_name = "
        "(SELECT name FROM player WHERE id = created_for) "
        "WHERE created_for IS NOT null AND created_for_name IS null");
      sneezy.query(
        "UPDATE immortal_exchange_coin SET redeemed_by_name = "
        "(SELECT name FROM player WHERE id = redeemed_by) "
        "WHERE redeemed_by IS NOT null AND redeemed_by_name IS null");
      sneezy.query(
        "UPDATE immortal_exchange_coin SET redeemed_for_name = "
        "(SELECT name FROM player WHERE id = redeemed_for) "
        "WHERE redeemed_for IS NOT null AND redeemed_for_name IS null");

      // Add indexes on player ID columns (created before FKs so FK
      // creation doesn't auto-create indexes with system-generated names)
      if (!hasIndex(sneezy, "immortal_exchange_coin", "idx_iec_created_by"))
        assert(
          sneezy.query("ALTER TABLE immortal_exchange_coin "
                       "ADD INDEX idx_iec_created_by (created_by)"));
      if (!hasIndex(sneezy, "immortal_exchange_coin", "idx_iec_created_for"))
        assert(
          sneezy.query("ALTER TABLE immortal_exchange_coin "
                       "ADD INDEX idx_iec_created_for (created_for)"));
      if (!hasIndex(sneezy, "immortal_exchange_coin", "idx_iec_redeemed_by"))
        assert(
          sneezy.query("ALTER TABLE immortal_exchange_coin "
                       "ADD INDEX idx_iec_redeemed_by (redeemed_by)"));
      if (!hasIndex(sneezy, "immortal_exchange_coin", "idx_iec_redeemed_for"))
        assert(
          sneezy.query("ALTER TABLE immortal_exchange_coin "
                       "ADD INDEX idx_iec_redeemed_for (redeemed_for)"));

      // Add FKs with ON DELETE SET null. When a player is deleted,
      // the ID becomes null but the name snapshot preserves who they were.
      addForeignKey(sneezy, "immortal_exchange_coin", "created_by", "player",
        "id", "SET null");
      addForeignKey(sneezy, "immortal_exchange_coin", "created_for", "player",
        "id", "SET null");
      addForeignKey(sneezy, "immortal_exchange_coin", "redeemed_by", "player",
        "id", "SET null");
      addForeignKey(sneezy, "immortal_exchange_coin", "redeemed_for", "player",
        "id", "SET null");
    },

    // Purge old shop log and journal archive entries. These two tables
    // account for 88% of database size (~680 MB) and grow without bound.
    // procShopLogCleanup handles ongoing retention; this clears the
    // initial backlog so it doesn't take days of periodic ticks.
    [&]() {
      vlogf(LOG_MISC, "Purging old shop log and journal archive entries");

      // Delete in batches to avoid oversized undo log
      for (;;) {
        sneezy.query(
          "DELETE FROM shoplog "
          "WHERE logtime < DATE_SUB(NOW(), INTERVAL 365 DAY) "
          "LIMIT 100000");
        if (sneezy.rowCount() == 0) {
          break;
        }
      }

      for (;;) {
        sneezy.query(
          "DELETE FROM shoplogjournalarchive "
          "WHERE logtime < DATE_SUB(NOW(), INTERVAL 365 DAY) "
          "LIMIT 100000");
        if (sneezy.rowCount() == 0) {
          break;
        }
      }
    },

    // Add FK constraints to world-data tables (mob.vnum, obj.vnum,
    // room.vnum). Enabled by the low mv* UPSERT refactor - parent rows
    // are no longer deleted during approve, so RESTRICT/CASCADE FKs are
    // safe.
    [&]() {
      vlogf(LOG_MISC, "Adding FK constraints to world-data tables");

      // Clean up orphaned roomexit.destination rows (exits to
      // non-existent rooms in disabled zones, plus one broken lift
      // exit with destination=-1).
      sneezy.query(
        "DELETE re FROM roomexit re "
        "LEFT JOIN room r ON re.destination = r.vnum "
        "WHERE r.vnum IS null");

      // CASCADE FK tables - delete orphans
      sneezy.query(
        "DELETE FROM trophy WHERE mobvnum NOT IN (SELECT vnum FROM mob)");
      sneezy.query(
        "DELETE FROM quest_limbs WHERE mob_vnum NOT IN (SELECT vnum FROM mob)");
      sneezy.query(
        "DELETE FROM board_message WHERE board_vnum NOT IN (SELECT vnum FROM "
        "obj)");
      sneezy.query(
        "DELETE FROM factoryblueprint WHERE vnum NOT IN (SELECT vnum FROM "
        "obj)");
      sneezy.query(
        "DELETE FROM shopproducing WHERE producing NOT IN "
        "(SELECT vnum FROM obj)");
      sneezy.query(
        "DELETE FROM factoryproducing WHERE vnum NOT IN "
        "(SELECT vnum FROM obj)");
      sneezy.query(
        "DELETE FROM shopownedratios WHERE obj_nr NOT IN "
        "(SELECT vnum FROM obj)");

      // Nullable RESTRICT FK columns - set null
      sneezy.query(
        "UPDATE property SET key_vnum=null WHERE key_vnum IS NOT null AND "
        "key_vnum NOT IN (SELECT vnum FROM obj)");
      sneezy.query(
        "UPDATE property SET entrance=null WHERE entrance IS NOT null AND "
        "entrance NOT IN (SELECT vnum FROM room)");

      // Small RESTRICT FK tables - delete orphans (broken data)
      sneezy.query("DELETE FROM pet WHERE vnum NOT IN (SELECT vnum FROM mob)");
      sneezy.query(
        "DELETE sd FROM ship_destinations sd LEFT JOIN mob m ON sd.vnum = "
        "m.vnum WHERE m.vnum IS null");
      sneezy.query(
        "DELETE sm FROM ship_master sm LEFT JOIN mob m ON sm.captain_vnum = "
        "m.vnum WHERE m.vnum IS null");
      sneezy.query(
        "DELETE sd FROM ship_destinations sd LEFT JOIN room r ON sd.room = "
        "r.vnum WHERE r.vnum IS null");

      // -> mob.vnum
      addForeignKey(sneezy, "shop", "keeper", "mob", "vnum", "RESTRICT");
      addForeignKey(sneezy, "trophy", "mobvnum", "mob", "vnum", "CASCADE");
      addForeignKey(sneezy, "pet", "vnum", "mob", "vnum", "RESTRICT");
      addForeignKey(sneezy, "quest_limbs", "mob_vnum", "mob", "vnum",
        "CASCADE");
      addForeignKey(sneezy, "ship_destinations", "vnum", "mob", "vnum",
        "RESTRICT");
      addForeignKey(sneezy, "ship_master", "captain_vnum", "mob", "vnum",
        "RESTRICT");

      // -> obj.vnum
      addForeignKey(sneezy, "board_message", "board_vnum", "obj", "vnum",
        "CASCADE");
      addForeignKey(sneezy, "factoryblueprint", "vnum", "obj", "vnum",
        "CASCADE");
      addForeignKey(sneezy, "property", "key_vnum", "obj", "vnum", "RESTRICT");
      addForeignKey(sneezy, "fishlargest", "vnum", "obj", "vnum", "CASCADE");
      addForeignKey(sneezy, "shopproducing", "producing", "obj", "vnum",
        "CASCADE");
      addForeignKey(sneezy, "rent", "vnum", "obj", "vnum", "RESTRICT");
      addForeignKey(sneezy, "factoryproducing", "vnum", "obj", "vnum",
        "CASCADE");
      addForeignKey(sneezy, "shopownedratios", "obj_nr", "obj", "vnum",
        "CASCADE");

      // -> room.vnum
      addForeignKey(sneezy, "shop", "in_room", "room", "vnum", "RESTRICT");
      addForeignKey(sneezy, "property", "entrance", "room", "vnum", "RESTRICT");
      addForeignKey(sneezy, "roomexit", "destination", "room", "vnum",
        "RESTRICT");
      addForeignKey(sneezy, "ship_destinations", "room", "room", "vnum",
        "RESTRICT");

      // -> zone.zone_nr (make nullable, convert -1 sentinel to null)
      modifyColumnType(sneezy, "room", "zone", "int(11)", false);
      modifyColumnType(immortal, "room", "zone", "int(11)", false);
      sneezy.query("UPDATE room SET zone = null WHERE zone = -1");
      immortal.query("UPDATE room SET zone = null WHERE zone = -1");
      addForeignKey(sneezy, "room", "zone", "zone", "zone_nr", "RESTRICT");
      addCrossDbForeignKey(immortal, "room", "zone", "sneezy", "zone",
        "zone_nr", "RESTRICT");
    },

    // Data cleanup and schema fixes from database audit
    [&]() {
      vlogf(LOG_MISC, "Database audit: cleanup and schema fixes");

      // Orphaned mail rent rows - attachments from deleted mail
      sneezy.query(
        "DELETE FROM rent WHERE owner_type = 'mail' "
        "AND rent_id NOT IN "
        "(SELECT rent_id FROM mail WHERE rent_id IS NOT null)");

      // Fix rent.container DEFAULT (-1 is wrong, code uses 0)
      sneezy.query("ALTER TABLE rent ALTER COLUMN container SET DEFAULT 0");

      // Convert mob float(5,1) to decimal(5,1) for exact game balance values
      for (const auto* col : {"attacks", "ac", "hpbonus", "damage_level"}) {
        if (!columnIsType(sneezy, "mob", col, "decimal(5,1)", true)) {
          sneezy.query("ALTER TABLE mob MODIFY %s DECIMAL(5,1) NOT null", col);
        }
        if (!columnIsType(immortal, "mob", col, "decimal(5,1)", true)) {
          immortal.query("ALTER TABLE mob MODIFY %s DECIMAL(5,1) NOT null",
            col);
        }
      }

      // Delete orphaned players with no account (legacy/test data)
      sneezy.query("DELETE FROM player WHERE account_id IS null");
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
