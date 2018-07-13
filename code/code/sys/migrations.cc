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
        assert(sneezy.query("select value from configuration where config = 'version'"));
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
                    "add column multiplay_limit int null default 3"
                    ));
            assert(sneezy.query(
                    "update account "
                    "set multiplay_limit = 3"
                    ));
            assert(sneezy.query(
                    "alter table account "
                    "change column multiplay_limit int not null default 3"
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
    };

    int oldVersion = getVersion(sneezy);
    int newVersion = migrations.size();

    vlogf(LOG_MISC, boost::format("Running migrations %d -> %d") % oldVersion % newVersion);
    for (int i = oldVersion; i < newVersion; ++i)
        migrations.at(i)();

    assert(sneezy.query("update configuration set value = '%i' where config = 'version'", newVersion));
    vlogf(LOG_MISC, "Migrations done");
}
