#!/bin/bash
# Creates and seeds the sneezy and immortal databases for testing or local dev.
# Usage: setup_db.sh [db_user]
#   db_user  MariaDB user to create and grant access (default: current OS user)

set -euo pipefail

DB_USER="${1:-$(whoami)}"

if [[ "$DB_USER" == *"'"* ]]; then
  echo "Error: DB_USER contains a single quote, which is not supported." >&2
  exit 1
fi

sudo systemctl is-active --quiet mariadb || sudo systemctl start mariadb
sudo mariadb -e "CREATE DATABASE IF NOT EXISTS sneezy; CREATE DATABASE IF NOT EXISTS immortal;"
sudo mariadb -e "CREATE USER IF NOT EXISTS '${DB_USER}'@'localhost'; GRANT ALL ON sneezy.* TO '${DB_USER}'@'localhost'; GRANT ALL ON immortal.* TO '${DB_USER}'@'localhost';"

if [ ! -d "_Setup-data" ]; then
  echo "Warning: _Setup-data/ not found. Databases created but not seeded." >&2
  echo "  Run this script from the repository root to seed game data." >&2
  exit 0
fi

for db in immortal sneezy; do
  for phase in tables views data; do
    [ -d "_Setup-data/sql_${phase}/${db}" ] || continue
    shopt -s nullglob
    for sql in "_Setup-data/sql_${phase}/${db}"/*.sql; do
      echo "loading '${sql}'"
      mariadb "${db}" < "${sql}"
    done
    shopt -u nullglob
  done
done
