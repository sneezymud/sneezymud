#include "database.h"
#include "log.h"
#include "migrations.h"

#include "charfile.h"
#include "extern.h" // for load_char

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
            sneezy.query("create table configuration (id int primary key auto_increment not null, config varchar(100) unique not null, value varchar(999) null)");
            sneezy.query("create unique index idx_configuration_key on configuration (config)");
            sneezy.query("insert into configuration (config, value) values ('version', '0')");
        },
        [&](){
            vlogf(LOG_MISC, "Migrating aliases to DB");
            sneezy.query(
                    "create table alias ("
                    "id int primary key auto_increment not null, "
                    "player_id bigint(20) unsigned not null, "
                    "word varchar(50) not null, "
                    "command varchar(999) not null, "
                    "foreign key (player_id) references player (id) on delete cascade)");

            sneezy.query("select id, name from player");
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
                        sneezy.query("insert into alias (player_id, word, command) values (%i, '%s', '%s')", id, alias.word, alias.command);
                    }
                }
            }
        },
        [&](){
            //support tweaks db
            vlogf(LOG_MISC, "Adding tweak table to DB");
            sneezy.query(
                    "create table globaltweaks ("
                    "tweak_id int primary key auto_increment not null, "
                    "tweak_type int not null, "
                    "tweak_value float(20) not null, "
                    "tweak_target float(20) not null, "
                    "tweak_rate float(20) not null, "
                    "datecreated datetime not null default CURRENT_TIMESTAMP)"
                    );

            //this doesn't work / migrations run before stats load      
            //sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values ('1','%f','%f','%f')", stats.equip, stats.global_lp_target, stats.global_lp_target_changerate);
            //another way would be to parse the stats file for current stats.equip value etc., but maybe an admin can just set the value after migration...
            
            //imm says stats.equip is 112.5
            sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (1,112.5,0.7,0.0)");//loadrate

            //burnrate and freezerate are 1.0 anyway.
            //sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (2,1.0,1.0,0.0)");//burnrate
            //sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (3,1.0,1.0,0.0)");//freezedamrate
        },
    };

    int oldVersion = getVersion(sneezy);
    int newVersion = migrations.size();

    vlogf(LOG_MISC, boost::format("Running migrations %d -> %d") % oldVersion % newVersion);
    for (int i = oldVersion; i < newVersion; ++i)
        migrations.at(i)();

    sneezy.query("update configuration set value = '%i' where config = 'version'", newVersion);
    vlogf(LOG_MISC, "Migrations done");
}
