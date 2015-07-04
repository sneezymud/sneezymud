SneezyMUD is opensource! See LICENSE.txt for details.

# -h|--help)

```
Usage: sneezy [-p PORT] [-l LIBDIR] [-c CONFIG]

Run sneezy, logging to stdout. Ctrl-C to exit.

    -p PORT     listen for Telnet connections on PORT
    -l LIBDIR   use LIBDIR as the lib flatfiles directory
    -c CONFIG   read configuration from CONFIG
```

## Defaults

| Telnet port               | 7900 |
| Config file (optional)    | `./sneezy.cfg` |
| Lib directory             | `./lib` |
| Database names            | `sneezy` and `immortal` |
| Database hostname         | `localhost` |
| Database username         | none (see below) |
| Database password         | none |

Note: when no username is configured, Sneezy's MySQL username defaults to the
Unix account name of the current user, similar to the mysql commandline
program.

## Configuration

To change the database configuration from the defaults, you must create a
custom config file. Two example cfg files are located in `code/`. Copy one of
them to `sneezy.cfg` and edit as needed. If you put it in Sneezy's starting
directory, it will get loaded automatically, or you can specify it on the
command line.

**Note:** Port 5678 is special, and when Sneezy is run on this port it enables
various developer and beta features. Do not use this port for a production MUD
instance!

# Requirements

## Server Environment

* Modern Unix, probably Linux, typically Ubuntu
* MySQL server or equivalent, two databases, and a user with table-level
  table-level access. See below for the defaults for these.

## Build Dependencies

* C++ compiler, probably g++ -- Tested with version 4.7
* scons -- Tested with version 2.2.0 on Python 2.7
* libmysqlclient -- Tested with libmysqlclient 5.5.28
* libc-ares -- Tested with 1.9.0 and 1.10.0.
* Boost C++ library, with 'program-options', 'regex', and 'exception' modules
  -- Tested with 1.40, 1.50

## Recommended

* bash shell and sudo root access

# Installation

When these instructions refer to 'Sneezy', they are mainly specifically
referring to the sneezymud server daemon program.

Shell commands below are assumed to have started in the root of the source
code directory tree. Substitute any changes from the defaults you require, and
remove sudo if it's unnecessary. Parts of the command where you must
substitute arbitrary choices are represented as shell **$VARIABLES**.

## Compiling

If you need to change build flags, edit the file `code/SConstruct`.

  $ cd code
  # -j sets parallel compilation, nproc reports number of cpus available
  $ scons -j$(nproc)

This will output a `code/sneezy` binary, along with some .so files in
`code/objs/`. These .so files are **required**, and must be located in an
`objs/` dir relative to the directory Sneezy is started in.

## Installing The Binary

If you're copying `code/sneezy` to a different location, you must copy the .so
files in their `objs` dir along with it. The easiest thing is just to copy the
whole dir:

  $ cd code && cp -r sneezy objs $DEST

Optionally delete the extraneous .o files:

  $ rm objs/*.o objs/*/*.o

## /lib - Flat Files

The 'lib' dir, as it is known, contains various text and data files that
Sneezy reads and occasionally writes. To keep paths simple in the source code,
Sneezy changes directory to the lib dir on startup. By default, Sneezy looks
for a `lib/` subdir of the directory it was started in.

First you'll need to make the required empty directories, because git doesn't
store them (the .. part of the cmd only works in bash):

  $ cd lib && mkdir -p roomdata/saved corpses immortals \
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

If you're using the defaults (no username/pw), set `[username]` below to the
Unix account sneezy will be running as, and create a no-password user:

  $ sudo mysql -e "CREATE USER '$USERNAME'@'localhost'"

Without a password, anyone who can connect to MySQL can log in as this user.
If this is a problem, set a password for the database user instead:

  $ sudo mysql -e "CREATE USER '$USERNAME'@'localhost' IDENTIFIED BY '$PASSWORD'"

### Set Database Permissions

  $ sudo mysql -e "GRANT ALL on sneezy.* to '$USERNAME'@'localhost' ;" \
               -e "GRANT ALL on immortal.* to '$USERNAME'@'localhost' ;"

Technically sneezy itself only needs `SELECT, UPDATE, INSERT` permissions, if
you use some other user to create the tables and populate the initial
database.

### Initial SQL Data

The initial MUD data is contained in per-table `mysqldump` files, which
combine table creation and data insertion in the form of a series of valid SQL
statements. They are found in `_Setup-Data/sql_data`, in per-database
directories.  Just load these files directly into `mysql` (don't forget to
specify the database):

  $ for db in immortal sneezy ; do
      for sql in _Setup-Data/sql_data/$db/*.sql ; do
        echo ">>>> LOADING '$db'"
        sudo mysql $db < $sql
      done
    done

## Running

When run, Sneezy will print copious logs to stdout, and can be safely shut
down using ctrl-C.

Run Sneezy in the source tree using the defaults:

  $ code/sneezy

If you created a custom sneezy.cfg in code/:

  $ cd code && ./sneezy
