SneezyMUD is opensource! See LICENSE.txt for details.

[![Build Status](https://travis-ci.org/sneezymud/sneezymud.svg?branch=master)](https://travis-ci.org/sneezymud/sneezymud)

# -h|--help)

```
Usage: sneezy [-p PORT] [-l LIBDIR] [-c CONFIG]

Run sneezy, logging to stdout. Ctrl-C to exit.

    -p PORT     listen for Telnet connections on PORT
    -l LIBDIR   use LIBDIR as the lib flatfiles directory
    -c CONFIG   read configuration from CONFIG

SIGHUP, SIGINT, SIGTERM also initiate shutdown.
```

## Defaults

| Configuration             | Default                       |
|---------------------------|-------------------------------|
| Telnet port               | 7900                          |
| Config file (optional)    | `./sneezy.cfg`                |
| Lib directory             | `./lib`                       |
| Database names            | `sneezy` and `immortal`       |
| Database hostname         | `localhost`                   |
| Database username         | none (see below)              |
| Database password         | none                          |

Note: when no username is configured, Sneezy's MySQL username defaults to the
Unix account name of the current user, similar to the mysql commandline
program.

## Configuration

To change the database and runtime configuration from the defaults, you must create a
custom config file. Two example cfg files are located in `code/`. Copy one of
them to `sneezy.cfg` and edit as needed. If you put it in Sneezy's starting
directory, it will get loaded automatically, or you can specify it on the
command line.

# Requirements

## Server Environment

* Modern Unix, probably Linux, typically Ubuntu
* MySQL-compatible database server, two databases, and a user with table-level
  write access. See above for the defaults for these.

## Build Dependencies

* Modern C++ compiler with working C++14 support
  -- Tested with clang 3.8 and gcc 5, but the newer the better generally
* scons -- Tested with version 2.4.1 on Python 2.7
* libmysqlclient -- Tested with libmysqlclient 5.7.19
* Boost C++ library, with 'program-options', 'regex', and 'filesystem' modules
  -- Tested with 1.58

## Recommended

* bash shell and sudo root access

# Installation

For most people, the install script `install_server.sh` should be enough to
get you going. Run with no arguments to get some help instructions. You
must still currently create the needed databases, users, and permissions
however. Refer to the section on database setup below.

## Manual Installation

When these instructions refer to 'Sneezy', they are mainly specifically
referring to the sneezymud server daemon program.

Shell commands below are assumed to have started in the root of the source
code directory tree. Substitute any changes from the defaults you require, and
remove sudo if it's unnecessary. Parts of the command where you must
substitute arbitrary choices are represented as shell **$VARIABLES**.

## Compiling

Running the `scons` command in the `code` dir will start the build process:

    $ cd code
    $ scons -j$(nproc)

This will output a `code/sneezy` binary, along with some .so files in
`code/objs/`. These .so files are **required**, and must be located in an
`objs/` dir relative to the directory Sneezy is started in.

If you need to change build flags, some can be specified on the commandline,
and others by editing the file `code/SConstruct`.  For more information see
the scons help message:

    $ scons -h

### Debugging Builds

Setting debug=1 during a build enables a variety of runtime misbehavior
checkers. In particular, AddressSanitizer is configured mainly via the
`ASAN_OPTIONS` and `LSAN_OPTIONS` environment variables. Documentation:

https://github.com/google/sanitizers/wiki/AddressSanitizerFlags

## Installing The Binary

If you're copying `code/sneezy` to a different location, you must copy the .so
files in their `objs` dir along with it. The easiest thing is just to copy the
whole dir:

    $ cd code && cp -r sneezy objs $DEST

Optionally delete the extraneous .o files:

    $ rm objs/*.o objs/*/*.o

## /lib - Flat Files

The 'lib' dir, as it is known, contains various text and data files that
Sneezy reads and occasionally writes. Files written to by the game are in
`lib/mutable`, whereas anything outside that dir is only ever read. This
structure is important for the Docker build to function correctly due to
the way Docker volumes work.

To keep paths simple in the source code, Sneezy changes directory to the
lib dir on startup. By default, Sneezy looks for a `lib/` subdir of the
directory it was started in.

First you'll need to make the required empty directories, because git doesn't
store them (the .. part of the cmd only works in bash):

    $ mkdir -p lib/mutable && cd lib/mutable && mkdir -p roomdata/saved immortals \
        corpses/corrupt rent/corrupt player/corrupt \
        rent/{a..z} account/{a..z} player/{a..z}

If you are planning to run Sneezy directly from the source tree, you're done.
Otherwise, copy the lib directory to its new location:

    $ cp -r lib $DEST/lib

## Database Setup

Sneezy uses both MySQL and flat files to store data, flat files being a
holdover from its early MUD roots. It connects to MySQL over TCP, so make sure
your database server's `bind-address` config option is set to `localhost` or
`127.0.0.1`.

### Create Databases

The names can be changed in the config file:

    $ sudo mysql -e "CREATE DATABASE sneezy ; CREATE DATABASE immortal ;"

### Create User

If you're using the defaults (no username/pw), set `$USERNAME` below to the
Unix account sneezy will be running as, and create a no-password user:

    $ sudo mysql -e "CREATE USER '$USERNAME'@'localhost'"

Without a password, anyone who can connect to MySQL can log in as this user.
If this is a problem, set a password for the database user instead:

    $ sudo mysql -e "CREATE USER '$USERNAME'@'localhost' IDENTIFIED BY '$PASSWORD'"

### Set Database Permissions

    $ sudo mysql -e "GRANT ALL on sneezy.* to '$USERNAME'@'localhost' ;" \
                -e "GRANT ALL on immortal.* to '$USERNAME'@'localhost' ;"

Technically Sneezy itself (probably) only needs `SELECT, UPDATE, INSERT`
permissions, if you use some other user to create the tables and populate the
initial database.

### Initial SQL Data

The initial database contents are loaded from files containing valid SQL, one
per file. They are in MySQL dialect (mostly originating from `mysqldump`) and
can be loaded simply by piping them into the `mysql` command. The files are
grouped by three main phases, which must be loaded in order:

1. Table creation (`sql_tables`)
2. View creation (`sql_views`)
3. Data insertion (`sql_data`)

Since Sneezy uses two databases, they are further divided by which database
they belong to, into `sneezy` and `immortal` dirs. So, we end up with the
following very complicated chunk of shell code to load it all correctly:

    $ for db in immortal sneezy ; do
        for phase in tables views data ; do
            [ -d "_Setup-data/sql_$phase/$db" ] || continue
            for sql in _Setup-data/sql_$phase/$db/*.sql ; do
                echo "loading '$sql'"
                mysql $db < $sql
            done
        done
    done


## Running

When run, Sneezy will print copious logs to stdout, and can be safely shut
down using ctrl-C.

Run Sneezy in the source tree using the defaults:

    $ code/sneezy

If you created a custom sneezy.cfg in code/:

    $ cd code && ./sneezy

# Database Migration

Ongoing changes to the database structure are stored as numbered migrations in
the `_Sql-data/migrations/` dir, one dir per migration. The naming convention
should be obvious. The SQL statements are stored in one of `immortal.sql` or
`sneezy.sql`, depending on which database they are for.

Migrations that have been applied to the initial data contained in this
repository are located in `_Sql-data/migrations/applied/` and do not need to
be applied to a fresh installation.

**Note:** there is currently no provision for migrating the _contents_ of
the databases, such as edits to room descriptions.
