# Database seed data

SQL dump files used to bootstrap new SneezyMUD server instances. A new server
will not function without this data - it contains all world definitions (rooms,
mobs, objects, shops, zones) and economy configuration.

## Setting up a local database

`db/init-db.sh` creates a MariaDB user for your OS account, drops and
recreates both databases, and loads all seed data. Run it from the repo root:

```sh
db/init-db.sh            # grants access to your current OS user
db/init-db.sh myuser     # or specify a different MariaDB user
```

The Docker database image has its own loading script that does the same thing
during container initialization.

## Updating seed data from production

When world data changes on the live server (builders editing rooms/mobs/shops,
schema migrations, etc.), regenerate the seed files from a production backup:

```sh
scripts/restore-backup-dev.sh    # seed local DB from latest production backup
db/update-seed-data.sh           # sanitize, dump, restore
```

The update script sanitizes player state, resets the economy to bootstrap
defaults, dumps to `db/`, and then restores your local database to its
original state. Supports `--dry-run` to preview what it would do.

## How the files are organized

```
db/
  sneezy/                One .sql file per table in the sneezy database
  immortal/              One .sql file per table in the immortal database (all schema-only)
  init-db.sh             Sets up local databases from seed data
  update-seed-data.sh    Regenerates seed data from a local database
```

Each `.sql` file is a self-contained `mariadb-dump` output. Seed tables
(listed in `SNEEZY_SEED_TABLES` in the update script) contain `CREATE TABLE` +
`INSERT` statements. All other tables contain `CREATE TABLE` only - they hold
player data, accounts, and other runtime state that starts empty on a fresh
server.

The immortal database is a builder staging area; all its tables are schema-only.
Loading order doesn't matter - each file handles its own `DROP TABLE IF EXISTS`
and foreign key checks.
