#!/bin/bash
# Initializes the sneezy and immortal databases from seed data.
# Drops and recreates both databases to ensure a clean state.
# Usage: init-db.sh [db_user]
#   db_user  MariaDB user to create and grant access (default: current OS user)

set -euo pipefail

DB_USER="${1:-$(whoami)}"

if [[ "$DB_USER" == *"'"* ]]; then
  echo "Error: DB_USER contains a single quote, which is not supported." >&2
  exit 1
fi

sudo systemctl is-active --quiet mariadb || sudo systemctl start mariadb
sudo mariadb -e "DROP DATABASE IF EXISTS immortal; DROP DATABASE IF EXISTS sneezy;"
sudo mariadb -e "CREATE DATABASE sneezy; CREATE DATABASE immortal;"
sudo mariadb -e "CREATE USER IF NOT EXISTS '${DB_USER}'@'localhost'; GRANT ALL ON sneezy.* TO '${DB_USER}'@'localhost'; GRANT ALL ON immortal.* TO '${DB_USER}'@'localhost';"

DB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

for db in immortal sneezy; do
  [[ -d "${DB_DIR}/${db}" ]] || continue
  shopt -s nullglob
  for sql in "${DB_DIR}/${db}"/*.sql; do
    echo "loading '${sql}'"
    mariadb "${db}" < "${sql}"
  done
  shopt -u nullglob
done
