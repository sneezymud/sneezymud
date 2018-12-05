#include "database.h"
#include "log.h"
#include "migrations.h"

#include "charfile.h"
#include "extern.h" // for load_char

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
}

void runMigrations() {
    TDatabase sneezy(DB_SNEEZY);
    TDatabase immortal(DB_IMMORTAL);

    std::vector<std::function<void()>> migrations = {
        [&](){
            vlogf(LOG_MISC, "Adding configuration table");
            assert(sneezy.query("create table configuration (id int primary key auto_increment not null, config varchar(100) unique not null, value varchar(999) null)"));
            assert(sneezy.query("create unique index idx_configuration_key on configuration (config)"));
            assert(sneezy.query("insert into configuration (config, value) values ('version', '0')"));
        },
        [&](){
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
                        vlogf(LOG_MISC, format("%d/%s: %s -> %s") % id % name % alias.word % alias.command);
                        assert(sneezy.query("insert into alias (player_id, word, command) values (%i, '%s', '%s')", id, alias.word, alias.command));
                    }
                }
            }
        },
        [&](){
            //support tweaks db
            vlogf(LOG_MISC, "Adding tweak table to DB");
            assert(sneezy.query(
                    "create table globaltweaks ("
                    "tweak_id int primary key auto_increment not null, "
                    "tweak_type int not null, "
                    "tweak_value float(20) not null, "
                    "tweak_target float(20) not null, "
                    "tweak_rate float(20) not null, "
                    "datecreated datetime not null default CURRENT_TIMESTAMP)"
                    ));
        },
        [&](){
            // configurable multiplay limit per account
            vlogf(LOG_MISC, "Adding multiplay column to account table");
            assert(sneezy.query(
                    "alter table account "
                    "add column multiplay_limit int null default 2"
                    ));
            assert(sneezy.query(
                    "update account "
                    "set multiplay_limit = 3"
                    ));
            assert(sneezy.query(
                    "alter table account "
                    "modify column multiplay_limit int not null default 2"
                    ));
        },
        [&](){
            vlogf(LOG_MISC, "Renaming Test Code 6 into DB Logging");
            assert(sneezy.query(
                    "update globaltoggles "
                    "set name = 'DB Logging', descr = 'log all db queries', testcode = 0 "
                    "where tog_id = 17"
                    ));
        },
        [&](){
            vlogf(LOG_MISC, "Adding savedrooms table");
            assert(sneezy.query(
                    "create table savedrooms ("
                    "id int primary key auto_increment not null, "
                    "player_id bigint(20) unsigned not null, "
                    "name varchar(50) not null, "
                    "room int not null, "
                    "foreign key (player_id) references player (id) on delete cascade)"));
        },
        [&](){
            vlogf(LOG_MISC, "Tying saved rooms to accounts");
            assert(sneezy.query("drop table if exists savedroomsacct"));
            assert(sneezy.query(
                    "create table savedroomsacct ("
                    "id int primary key auto_increment not null, "
                    "account_id bigint(20) unsigned not null, "
                    "name varchar(50) not null, "
                    "room int not null, "
                    "foreign key (account_id) references account (account_id) on delete cascade)"));
            assert(sneezy.query(
                    "insert into savedroomsacct select "
                    "s.id, a.account_id, s.name, s.room "
                    "from savedrooms s join player p "
                    "on s.player_id = p.id "
                    "join account a on p.account_id = a.account_id "));
            assert(sneezy.query("drop table savedrooms"));
        },
        [&](){
            vlogf(LOG_MISC, "Adding generic per-account and per-player storage");
            assert(sneezy.query(
                    "create table if not exists accountnotes ("
                    "id int primary key auto_increment not null, "
                    "account_id bigint(20) unsigned not null, "
                    "name varchar(64) not null, "
                    "value text not null, "
                    "foreign key (account_id) references account (account_id) on delete cascade)"));
            assert(sneezy.query(
                    "create table if not exists playernotes ("
                    "id int primary key auto_increment not null, "
                    "player_id bigint(20) unsigned not null, "
                    "name varchar(64) not null, "
                    "value text not null, "
                    "foreign key (player_id) references player (id) on delete cascade)"));
        },
        [&](){
            vlogf(LOG_MISC, "Moving wiz data over to db");
            assert(sneezy.query(
                    "create table if not exists wizdata ("
                    "setsev int not null,"
                    "office int not null,"
                    "blockastart int not null,"
                    "blockaend int not null,"
                    "blockbstart int not null,"
                    "blockbend int not null,"
                    "player_id bigint(20) unsigned not null, "
                    "primary key (player_id), "
                    "foreign key (player_id) references player (id) on delete cascade)"));
            assert(sneezy.query("select id, name from player"));
            std::map<int, std::string> idToName;
            while (sneezy.fetchRow())
                idToName[convertTo<int>(sneezy["id"])] = sneezy["name"];
            class wizSaveData {
            public:
                int setsev,
                    office,
                    blockastart,
                    blockaend,
                    blockbstart,
                    blockbend;
            };
            for (auto& player : idToName) {
                const auto id = player.first;
                const auto& name = player.second;
                FILE *fp;
                sstring buf, buf2;
                wizSaveData saveData;
                
                buf = format("immortals/%s/wizdata") % name;
                fp = fopen(buf.c_str(), "r");
                if (!fp) {
                    continue;
                }
                if (fread(&saveData, sizeof(saveData), 1, fp) != 1) {
                    vlogf(LOG_BUG, format("Corrupt wiz save file for %s") % name);
                    fclose(fp);
                    continue;
                } 
                fclose(fp);
                assert(sneezy.query("insert into wizdata (setsev, office, blockastart, blockaend, blockbstart, blockbend, player_id) "
                                    "values (%i, %i, %i,%i, %i, %i, %i)", saveData.setsev, saveData.office, saveData.blockastart, 
                                    saveData.blockaend, saveData.blockbstart, saveData.blockbend, id));
            }     
        },
    };

    int oldVersion = getVersion(sneezy);
    int newVersion = migrations.size();

    vlogf(LOG_MISC, boost::format("Running migrations %d -> %d") % oldVersion % newVersion);
    for (int i = oldVersion; i < newVersion; ++i)
        migrations.at(i)();

    assert(sneezy.query("update configuration set value = '%i' where config = 'version'", newVersion));
    vlogf(LOG_MISC, "Migrations done");
}
