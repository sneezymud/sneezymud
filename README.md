This is SneezyMUD.

# Prerequisites

* gcc, namely g++ -- I'm using version 4.7
* scons -- I'm using version 2.2.0 on Python 2.7
* MySQL server and libmysqlclient. You need at least one database, "sneezymud",
  better also "sneezybeta". Maybe more, I'm still figuring it out. The username
  is hardcoded to be "sneezy" -- I'm using libmysqlclient 5.5.28-1
* gperftools -- I'm using 2.0. This one is not absolutely necessary. If you
  don't have it, just remove -ltcmalloc from code/SConstruct.
* c-ares -- mine is 1.9.0. I've had compatibility issues, there seem to be
  different versions around with different APIs. The adaptation to mine was
  easy, though.
* Boost -- Tried 1.40.0 and 1.50.0

# Installation

TODO: create simpler, more explicit instructions

## Compiling

  cd sneezymud/code
  scons -j2
  (2 is the number of parallel compilation jobs to run)

## Database

* Sneezy connects to MySQL over TCP, so enable this.

* Create database sneezy: create database sneezy;

* Create user sneezy and grant some privileges on sneezy db: CREATE USER 'sneezy'@'localhost' IDENTIFIED BY '$uper$ecret'; GRANT ALL ON sneezy.* TO 'sneezy@'localhost';

* Import tables:

  In code/sql, run the same stuff as below.  In data/immortal, run this:

    for i in *;do echo "Importing $i"; mysql -u sneezy immortal < $i;done

  In data/sneezy:

    for i in *;do echo "Importing $i"; mysql -u sneezy sneezy < $i;done

* Create directories (use Bash for this, ZSH doesn't work):

  cd lib
  mkdir -p roomdata/saved
  mkdir rent
  cd rent
  for i in {a..z};do mkdir $i;done
  cd ..
  mkdir account
  cd account
  for i in {a..z};do mkdir $i;done
  cd ..
  mkdir player
  cd player
  for i in {a..z};do mkdir $i;done
  cd ..
  mkdir corpses
  mkdir -p mobdata/repairs
  mkdir immortals
  cd ..

* Create a configuration:

  Edit the configuration sneezy.cfg to point into the correct lib/ directory.

## Running

start with ./sneezy
It'll listen on port 5678 (if you're using sneezybeta), or 7900 otherwise.

# Running multiple instances in parallel

You can configure different database names, users and passwords for
different instances: add sneezy_db = my_sneezy, sneezy_user = my_user,
sneezy_password = my_password, port=7902 to sneezy.cfg.
