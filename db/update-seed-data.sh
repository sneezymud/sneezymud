#!/bin/bash
#
# Produce canonical seed data for fresh SneezyMUD server instances.
#
# This script sanitizes the local database (removing player state, resetting
# the economy to bootstrap defaults) and then exports it to db/ as SQL dump
# files. The database is backed up before sanitization and restored afterward,
# so your local data is never permanently modified.
#
# Run this against a dev database seeded from a production backup, never
# against the live server - sanitization mutates tables in place while it runs.
#
# Each table gets a single .sql file under db/{database}/. Seed tables
# (listed in SNEEZY_SEED_TABLES below) get schema + row data. All other
# tables get schema only. The immortal database is entirely schema-only
# (it's a builder staging area for in-progress edits, not canonical world
# definitions).
#
# Output is deterministic: timestamps, version headers, and runtime
# AUTO_INCREMENT counters are stripped so git diffs reflect only real
# schema or data changes. We intentionally don't record the MariaDB
# version - the dump format's /*!NNNNN ...*/ conditional comments are
# version gates evaluated at import time, so dumps are cross-version
# compatible by design.
#
# Run this after:
#   - Restoring a production backup (scripts/restore-backup-dev.sh)
#   - Migrations that change schema or seed data tables
#   - World data changes by builders (rooms, mobs, objects, shops)
#
# Usage:
#   db/update-seed-data.sh              # sanitize + dump (local database)
#   db/update-seed-data.sh --docker     # target Docker dev environment
#   db/update-seed-data.sh --dry-run    # show what would happen

set -euo pipefail

DB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DRY_RUN=false
DOCKER_MODE=false
DB_CONTAINER="sneezy-db"
DB_USER="sneezy"
DB_PASS="password"

for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=true ;;
    --docker)  DOCKER_MODE=true ;;
    *)         echo "Unknown option: $arg" >&2; exit 1 ;;
  esac
done

if $DOCKER_MODE; then
  if ! docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "$DB_CONTAINER"; then
    echo "Error: Docker container '$DB_CONTAINER' is not running." >&2
    echo "  Start it with: docker compose up -d db" >&2
    exit 1
  fi
fi

# Route mariadb/mariadb-dump commands through Docker when --docker is set.
# Usage: db_cmd mariadb [args...]       - execute with args
#        db_cmd mariadb [args...] < file - pipe stdin (adds -i for docker exec)
db_cmd() {
  if $DOCKER_MODE; then
    local -a cmd=(docker exec)
    [[ ! -t 0 ]] && cmd+=(-i)
    cmd+=("$DB_CONTAINER")
    local tool="$1"; shift
    "${cmd[@]}" "$tool" -u "$DB_USER" -p"$DB_PASS" "$@"
  else
    "$@"
  fi
}

# --- Seed table classification ---
#
# Seed data is the minimum needed to boot a playable server from scratch:
# static world definitions, economy configuration, and the migration version.
# Player state, runtime logs, and accumulated game data are excluded - those
# tables get schema-only dumps and start empty on fresh instances.
#
# New tables added by migrations default to schema-only. Add a table here
# only if a fresh server needs its rows to function. When a table is dropped,
# no changes needed - the stale file cleanup handles it automatically.

SNEEZY_SEED_TABLES=(
  # World definitions - the game world itself
  mob mob_extra mob_imm mobresponses
  obj objaffect objextra
  room roomexit roomextra
  zone
  shop shoptype shopmaterial shopproducing
  ship_master ship_destinations
  globaltoggles
  property

  # Economy infrastructure - corp/shop ownership and accounting structure.
  # Sanitization strips player corps, resets shop pricing to defaults,
  # and zeros corp banks.
  shopowned
  corporation
  shopownedcorpbank
  shoplogaccountchart
  shopownedrepair

  # Migration version counter. Without the correct version row, the server
  # re-runs all migrations against already-migrated schema. This must always
  # reflect the current migration count.
  configuration

  # Factory system - vestigial (one factory, zero supplies, zero output) but
  # valid world configuration. 5 total rows. Goes away if the feature does.
  factoryblueprint factoryproducing factorysupplies
)

# --- Helpers ---

# Strip table-level AUTO_INCREMENT=N (runtime counter, not schema).
# Column-level AUTO_INCREMENT keyword is unaffected (no '=' after it).
clean_dump() {
  sed 's/ AUTO_INCREMENT=[0-9]*//'
}

dump_one() {
  local db="$1" table="$2" mode="$3" outfile="$4"
  local flags=(--skip-comments --skip-dump-date)
  [[ "$mode" == "schema" ]] && flags+=(--no-data)

  if $DRY_RUN; then
    echo "  would dump: $db.$table ($mode) -> $(basename "$outfile")"
    return
  fi

  db_cmd mariadb-dump "${flags[@]}" "$db" "$table" | clean_dump > "$outfile"
}

run_sql() {
  local db="$1" sql="$2"
  if $DRY_RUN; then
    echo "  would run on $db: $sql"
  else
    db_cmd mariadb "$db" -e "$sql"
  fi
}

# --- Sanitization ---
#
# Converts a production database snapshot into sterile seed data by removing
# player-specific state and resetting the economy to bootstrap defaults.
# The database is backed up before sanitization and restored afterward (or
# on failure/interrupt), so the local data is never permanently modified.

PLAYER_CORP_IDS="2,3,4,6,7,8,11,12,13,17,30"

BACKUP_DIR=$(mktemp -d)
trap 'restore_backup' EXIT

backup_databases() {
  if $DRY_RUN; then return; fi
  echo "=== Backing up databases ==="
  db_cmd mariadb-dump --single-transaction sneezy > "$BACKUP_DIR/sneezy.sql"
  db_cmd mariadb-dump --single-transaction immortal > "$BACKUP_DIR/immortal.sql"
  echo "  backed up to $BACKUP_DIR"
}

restore_backup() {
  if $DRY_RUN; then rm -rf "$BACKUP_DIR"; return; fi
  echo "=== Restoring databases ==="
  db_cmd mariadb sneezy < "$BACKUP_DIR/sneezy.sql"
  db_cmd mariadb immortal < "$BACKUP_DIR/immortal.sql"
  rm -rf "$BACKUP_DIR"
  echo "  databases restored to pre-sanitization state"
}

backup_databases

echo "=== Sanitizing database ==="

# Step 1: Reassign player-owned shops (those owned by player-owned corps) to
# game-owned corps.
#
# Must happen before deleting corps due to FK on shopowned.corp_id.
#
# Assignments are geographic - each shop goes to the corp that governs the zone
# it physically exists in or a zone its zone connects to.
#
# Any shops that don't exist in or next to a city get assigned to the SBA (Small
# Business Administration) corp, which has no city and serves as a catch-all for
# non-city shops.
echo "Reassigning player-owned shops to nearest towns..."
# To Grimhaven (corp 21): zones Market Place (159), Southern Market
# (113,114,115,117,118,119,120,121,135,137,139,143), GK Casino (122)
run_sql sneezy "UPDATE shopowned SET corp_id = 21
  WHERE shop_nr IN (113,114,115,117,118,119,120,121,122,135,137,139,143,159);"

# To Logrus realm (corp 28): zone Town of Logrus (173)
run_sql sneezy "UPDATE shopowned SET corp_id = 28 WHERE shop_nr = 173;"

# To SBA (corp 1): not adjacent to any city zone - Tequila Sunrise (13),
# Haunted Hobbit Village (178,181), Pleasant Acres (183), Faction
# Mansion (227,228,229,230), Viking Ship (255,256,257,258)
run_sql sneezy "UPDATE shopowned SET corp_id = 1
  WHERE shop_nr IN (13,178,181,183,227,228,229,230,255,256,257,258);"

# Transfer any shops owned by faction corps to their city corps.
# Faction corps (Galek, Logrus cult, Serpents) should exist but not own
# shops on a fresh server.
run_sql sneezy "UPDATE shopowned SET corp_id = 28 WHERE corp_id = 24;" # Cult of Logrus -> Logrus realm
run_sql sneezy "UPDATE shopowned SET corp_id = 29 WHERE corp_id = 23;" # Brotherhood of Galek -> Brightmoon
run_sql sneezy "UPDATE shopowned SET corp_id = 27 WHERE corp_id = 25;" # Order of Serpents -> Amber

# Step 2: Create shopowned entries for shops that have never had one.
# These shops exist in the shop table but were never configured with
# ownership. Use INSERT IGNORE so re-runs are safe.
echo "Creating missing shopowned entries..."
# Grimhaven Bank (123)
run_sql sneezy "INSERT IGNORE INTO shopowned (shop_nr, corp_id) VALUES (123, 21);"
# Logrus Shipping Office (216)
run_sql sneezy "INSERT IGNORE INTO shopowned (shop_nr, corp_id) VALUES (216, 28);"
# Everything else: Player Homes (225,254,259), Aerie Treehouse (249),
# Skyhall (253), Avalon Outpost (262,263)
run_sql sneezy "INSERT IGNORE INTO shopowned (shop_nr, corp_id)
  VALUES (225,1),(249,1),(253,1),(254,1),(259,1),(262,1),(263,1);"

# Step 3: Remove player-owned corporations
echo "Removing player-owned corporations..."
run_sql sneezy "DELETE FROM corpaccess WHERE corp_id IN ($PLAYER_CORP_IDS);"
run_sql sneezy "DELETE FROM corplog WHERE corp_id IN ($PLAYER_CORP_IDS);"
run_sql sneezy "DELETE FROM shopownedcorpbank WHERE corp_id IN ($PLAYER_CORP_IDS);"
run_sql sneezy "DELETE FROM corporation WHERE corp_id IN ($PLAYER_CORP_IDS);"

# Step 4: Reset shop economics to uniform defaults.
# Gold is set to the midpoint of the reserve range (1M). The reserve
# system (doReserve in shopowned.cc) automatically moves gold between
# shops and their corp bank to keep shops within [reserve_min, reserve_max].
# Starting at the midpoint means no immediate reserve transfers needed.
# Margins are reset to the standard 10% spread (buy at 1.1x, sell at 0.9x),
# replacing years of player customization.
echo "Sanitizing shop config..."
run_sql sneezy "UPDATE shopowned SET
  gold = 1000000,
  profit_buy = 1.1,
  profit_sell = 0.9,
  reserve_min = 900000,
  reserve_max = 1100000,
  dividend = NULL;"

# Step 5: Clear player-configured shop overrides
echo "Clearing player shop overrides..."
run_sql sneezy "DELETE FROM shopownedratios;"
run_sql sneezy "DELETE FROM shopownedmatch;"
run_sql sneezy "DELETE FROM shopownedplayer;"

# Step 6: Zero corporation bank balances.
# Corp banks are intentionally empty rather than pre-capitalized. On a
# fresh server, players sell loot far more than they buy, so popular
# shops near spawn drain their gold first. Without corp bank reserves
# to refill from, this forces players to travel to less-trafficked shops,
# naturally dispersing money across the game world. Once shops accumulate
# enough from sales to exceed reserve_max, doReserve() starts depositing
# to corp banks and the reserve system begins functioning normally.
echo "Zeroing corp bank balances..."
run_sql sneezy "UPDATE shopownedcorpbank SET talens = 0, earned_interest = 0;"

# Ensure every corp has a bank entry at its configured bank shop.
# getMoney() returns 0 and setMoney() silently no-ops if the row is
# missing, which breaks the reserve system. Production backups may be
# missing entries for corps that never deposited.
run_sql sneezy "INSERT IGNORE INTO shopownedcorpbank (shop_nr, corp_id, talens, earned_interest)
  SELECT c.bank, c.corp_id, 0, 0 FROM corporation c
  LEFT JOIN shopownedcorpbank cb ON c.corp_id = cb.corp_id AND c.bank = cb.shop_nr
  WHERE cb.corp_id IS NULL;"

# Step 7: Verify
echo "=== Verifying ==="

if ! $DRY_RUN; then
  unowned=$(db_cmd mariadb sneezy -N -e "SELECT COUNT(*) FROM shop s
    LEFT JOIN shopowned so ON s.shop_nr = so.shop_nr WHERE so.shop_nr IS NULL;")
  orphaned=$(db_cmd mariadb sneezy -N -e "SELECT COUNT(*) FROM shopowned s
    LEFT JOIN corporation c ON s.corp_id = c.corp_id WHERE c.corp_id IS NULL;")
  missing_bank=$(db_cmd mariadb sneezy -N -e "SELECT COUNT(DISTINCT s.corp_id) FROM shopowned s
    LEFT JOIN shopownedcorpbank cb ON s.corp_id = cb.corp_id WHERE cb.corp_id IS NULL;")
  no_reserves=$(db_cmd mariadb sneezy -N -e "SELECT COUNT(*) FROM shopowned
    WHERE reserve_min = 0 OR reserve_max = 0;")
  player_corps=$(db_cmd mariadb sneezy -N -e "SELECT COUNT(*) FROM corporation
    WHERE corp_id IN ($PLAYER_CORP_IDS);")
  fail=false
  [[ "$unowned" -ne 0 ]] && echo "FAIL: $unowned shops without shopowned entries" && fail=true
  [[ "$orphaned" -ne 0 ]] && echo "FAIL: $orphaned orphaned shops" && fail=true
  [[ "$missing_bank" -ne 0 ]] && echo "FAIL: $missing_bank corps missing bank entries" && fail=true
  [[ "$no_reserves" -ne 0 ]] && echo "FAIL: $no_reserves shops without reserves" && fail=true
  [[ "$player_corps" -ne 0 ]] && echo "FAIL: $player_corps player corps remain" && fail=true

  if $fail; then
    echo "Verification failed, aborting before dump."
    exit 1
  fi
  echo "All checks passed."
fi

# --- Dump ---

echo "=== Dumping ==="

declare -A is_seed_table
for t in "${SNEEZY_SEED_TABLES[@]}"; do
  is_seed_table[$t]=1
done

mkdir -p "$DB_DIR/sneezy" "$DB_DIR/immortal"

# Sneezy database
mapfile -t sneezy_tables < <(db_cmd mariadb -N -e "SHOW TABLES" sneezy)
echo "sneezy: ${#sneezy_tables[@]} tables"

seed_count=0
schema_count=0

for table in "${sneezy_tables[@]}"; do
  if [[ -n "${is_seed_table[$table]:-}" ]]; then
    dump_one sneezy "$table" data "$DB_DIR/sneezy/$table.sql"
    seed_count=$((seed_count + 1))
  else
    dump_one sneezy "$table" schema "$DB_DIR/sneezy/$table.sql"
    schema_count=$((schema_count + 1))
  fi
done

echo "  $seed_count with seed data, $schema_count schema-only"

remove_stale_files() {
  local db="$1" dir="$2"
  local -n tables="$3"
  for f in "$dir/"*.sql; do
    [[ -f "$f" ]] || continue
    local table="$(basename "$f" .sql)"
    local found=false
    for t in "${tables[@]}"; do
      [[ "$t" == "$table" ]] && { found=true; break; }
    done
    if ! $found; then
      if $DRY_RUN; then
        echo "  would remove dropped: $db/$table.sql"
      else
        rm "$f"
        echo "  removed dropped: $db/$table.sql"
      fi
    fi
  done
}

remove_stale_files sneezy "$DB_DIR/sneezy" sneezy_tables

# Immortal database (schema-only, builder staging area)
mapfile -t immortal_tables < <(db_cmd mariadb -N -e "SHOW TABLES" immortal)
echo "immortal: ${#immortal_tables[@]} tables (all schema-only)"

for table in "${immortal_tables[@]}"; do
  dump_one immortal "$table" schema "$DB_DIR/immortal/$table.sql"
done

remove_stale_files immortal "$DB_DIR/immortal" immortal_tables

echo "done."
