#include "database.h"
#include "log.h"
#include "migrations.h"

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
            sneezy.query("drop table if exists configuration");
            sneezy.query("create table configuration (id int auto_increment not null, config varchar(100) unique not null, value varchar(999) null, primary key(id))");

            sneezy.query("create unique index idx_configuration_key on configuration (config)");
            sneezy.query("insert into configuration (config, value) values ('version', '0')");
        },
    };

    int oldVersion = getVersion(sneezy);
    int newVersion = migrations.size();

    vlogf(LOG_MISC, boost::format("Running migrations %d -> %d") % oldVersion % newVersion);
    for (int i = oldVersion; i < newVersion; ++i)
        migrations.at(i)();

    sneezy.query("update configuration set value = '%i' where config = 'version'", newVersion);
    vlogf(LOG_MISC, "Migrations done");

    throw std::runtime_error("exit");
}
