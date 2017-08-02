#! /bin/bash

# Builds and installs Sneezymud in the specified dir and MySQL databases.
#
# TODO:
#   * interactive install mode
#   * non-debian os pkg dependencies
#   * library version checking
#   * create user and databases automatically
#   * add more useful commented-out cfg settings
#   * test inserts to temp table
#   * more thorough source lib tree checks

set -e
exec 3>&2 # save a copy

deb_builddeps () {
    cat <<EOF
libmysqlclient-dev
libc-ares-dev
g++
scons
libboost-dev
libboost-regex-dev
libboost-program-options-dev
EOF
}

usage () {
    cat <<EOF
Usage: $(basename "$0") [flags] DESTDIR

Installs a copy of SneezyMUD in DESTDIR, which cannot already exist.
SneezyMUD is built first if necessary. Requires full permissions on the two
given MySQL databases. The script will abort if they have data, unless -w
is used to wipe them instead.

Optional flags:
    -s SRCDIR   Directory containing the SneezyMUD source tree (defaults to
                the current directory).
    -d SNEEZYDB Database to load sneezymud data into. Defaults to 'sneezy'.
    -i IMMDB    Database for immort data. Defaults to 'immortal'.
    -u USER     Username to connect to mysql with.
    -n HOST     Name of mysql server to connect to. Defaults to 'localhost'.
    -p PASS     Password for connecting to mysql.
    -w          Wipe the database prior to setup.
    -j NUM      Run build with this many parallel processes. Defaults to the
                output of 'nproc'. Set to 1 to disable.
    -q          Quiet: only emit output on errors.

Some settings can also be overridden via these environment variables:

SNEEZY_DBUSER SNEEZY_DBPASS SNEEZY_DBHOST SNEEZY_SNEEZYDB SNEEZY_IMMDB
EOF
}

parse_args () {
    DBUSER="$SNEEZY_DBUSER"
    DBPASS="$SNEEZY_DBPASS"
    DBHOST="$SNEEZY_DBHOST"
    SNEEZYDB="${SNEEZY_SNEEZYDB:-sneezy}"
    IMMDB="${SNEEZY_IMMDB:-immortal}"
    SOURCE=
    QUIET=
    DO_DBWIPE=
    DO_PARALLEL="$(nproc)"

    while getopts "i:n:d:p:u:s:j:wqh" o; do
        case "$o" in
            i)  IMMDB="$OPTARG" ;;
            n)  DBHOST="$OPTARG" ;;
            d)  SNEEZYDB="$OPTARG" ;;
            p)  DBPASS="$OPTARG" ;;
            u)  DBUSER="$OPTARG" ;;
            s)  SOURCE="$OPTARG" ;;
            j)  DO_PARALLEL="$OPTARG" ;;
            w)  DO_DBWIPE=yes ;;
            q)  QUIET=yes ;;
            h)  usage ; exit 0 ;; # sekrit help flag
            *)  badusage ;; # getopts does its own bitching in this case
        esac
    done
    shift $((OPTIND-1))

    DEST="$1"
    [ "$DEST" ] || badusage "missing or empty DESTDIR argument"

    # this the _sane_ way to sanitize ints in bash
    DO_PARALLEL="$(printf %i "$DO_PARALLEL" 2>/dev/null || true)"
    # don't error out, just clamp it
    if [ $DO_PARALLEL -lt 2 ] ; then DO_PARALLEL=1 ; fi
    # reasonable ceiling to avoid surprise behavior
    if [ $DO_PARALLEL -gt 16 ] ; then DO_PARALLEL=16 ; fi
}

badusage () {
    exec >&2 # it's only errors from here on out
    if [ $# -gt 0 ] ; then
        echo "ERROR:" "$@"
        echo
    fi
    usage
    exit 1
}

fatal () { echo "ERROR:" "$@" >&3 ; exit 1 ;}
warn ()  { echo "WARN: " "$@" >&3 ;}
info ()  { if ! [ "$QUIET" ] ; then echo "INFO: " "$@" ; fi ;}
debug () { if [ "$DEBUG" ] ; then echo "DEBUG:" "$@" ; fi ;}

main () {
    debug start
    parse_args "$@"

    [ "$SOURCE" ] && cd "$SOURCE"

    # check phase - for minimizing errors during build phase
    info "starting check phase"
    [ -e "$DEST" ] && fatal "Destination dir '$DEST' already exists"
    check_lib
    check_code
    check_dbs
    # add any other important pre-install tests here

    info "starting build phase"
    if ! [ -x "code/sneezy" ] ; then
        info "building sneezy"
        ( cd code && scons ${DO_PARALLEL:+-j$DO_PARALLEL} )
        info "done building"
    fi

    info "starting install phase"
    setup_lib
    generate_config > "$DEST"/sneezy.cfg
    wipe_dbs
    load_dbs
    info "done! you're all set, just run ./sneezy in $DEST"
}

# ---------- check phase -----------
check_lib () {
    # probe dir tree structure
    require_file "lib/txt/motd"
    require_file "lib/zonefiles/0"
    require_file "_Setup-data/sql_tables/sneezy/mob.sql"
    require_file "_Setup-data/sql_tables/immortal/mob.sql"
    require_file "_Setup-data/sql_data/sneezy/mob.sql"
    require_file "_Setup-data/sql_data/immortal/mob.sql"
}

check_code () {
    [ -x "code/sneezy" ] && return
    if [ ! -f "code/SConstruct" ] ; then
        fatal "source code needed to build sneezy is missing"
    fi

    # dependency checking
    which c++ >/dev/null || fatal "no C++ compiler found, recommend g++"
}

check_dbs() {
    # check access to dbs and if they have contents
    info "checking mysql database access"
    local db=
    for db in $SNEEZYDB $IMMDB; do
        local tables="$(dbquery $db "show tables")"
        if [ "$tables" ] && [ "$DO_DBWIPE" != yes ] ; then
            fatal "Data already exists in db '$db' (use -w flag to wipe)"
        fi
        local t="temp_$(echo $RANDOM | md5sum | cut -d' ' -f1)"
        dbquery $db "create table \`$t\` (i int)" || fatal "Failed to create test table '$t' in db '$db'"
        dbquery $db "drop table \`$t\`" || fatal "Failed to drop test table '$t' in db '$db'"
    done
}

# -------------- build phase ------------

setup_lib () {
    mkdir -p "$DEST"
    # see if this is a build with the shared libs
    if [ -f code/objs/libsys.so ] ; then
        mkdir "$DEST"/objs
        cp code/objs/*.so "$DEST"/objs
    fi
    cp code/sneezy "$DEST"
    cp -r lib "$DEST"
    (
        cd "$DEST"/lib
        mkdir -p roomdata/saved immortals \
            corpses/corrupt rent/corrupt player/corrupt \
            rent/{a..z} account/{a..z} player/{a..z}
    )
}

generate_config_line () {
    [ "$1" ] && [ "$1" != "$3" ] && echo "$2=$1" || echo "#$2=$3"
}

generate_config () {
    generate_config_line ""          port               7900
    generate_config_line ""          lib                ./lib

    generate_config_line "$SNEEZYDB" sneezy_db          sneezy
    generate_config_line "$DBUSER"   sneezy_user        sneezy
    generate_config_line "$DBPASS"   sneezy_password
    generate_config_line "$DBHOST"   sneezy_host        localhost

    generate_config_line "$IMMDB"    immortal_db        immortal
    generate_config_line "$DBUSER"   immortal_user      sneezy
    generate_config_line "$DBPASS"   immortal_password
    generate_config_line "$DBHOST"   immortal_host      localhost
}

wipe_dbs () {
    [ "$DO_DBWIPE" != yes ] && return

    local db= tables=
    for db in $SNEEZYDB $IMMDB ; do
        info "wiping database '$db'"
        if ! wrapmysql -e "use $db" 2>/dev/null ; then
            debug "creating fresh database"
            wrapmysql -e "create database $db"
            continue
        fi
        tables="$(dbquery $db "show tables" | tail -n+2 | awk '{print $1}')"
        if ! [ "$tables" ]; then
            debug "empty database, skipping"
            continue
        fi
        if wrapmysql -e "drop database $db" 2>/dev/null ; then
            debug "drop/adding database"
            wrapmysql -e "create database $db"
            continue
        fi
        for table in $tables ; do
            debug "dropping '$table' from '$db'"
            dbquery $db "drop table \`$table\`"
        done
    done
}

load_dbs () {
    info "loading databases"
    local db= sql=
    for phase in sql_tables sql_views sql_data ; do
        for db in sneezy immortal ; do
            [ -d _Setup-data/"$phase"/"$db" ] || continue
            debug "loading $phase for '$db'"
            for sql in _Setup-data/"$phase"/"$db"/*.sql ; do
                debug "loading '$sql' into '$db'"
                dbquery $db "source $sql"
            done
        done
    done
    info "done loading databases"
}

# -------------- utils ----------------
dbquery () {
    [ "$1" ] && [ "$2" ] || fatal "weird dbquery call"
    wrapmysql "$1" -e "$2"
}

wrapmysql () {
    debug mysql "$@"
    mysql ${DBUSER:+-u$DBUSER} ${DBPASS:+-p$DBPASS} ${DBHOST:+-h$DBHOST} "$@"
}

require_path () { [ $1 "$2" ] || fatal "required $3 '$2' missing" ;}
require_file () { require_path -f "$1" "file" ;}
require_dir () { require_path -d "$1" "directory" ;}

main "$@"
