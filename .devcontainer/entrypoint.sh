#!/bin/bash
# Container entrypoint - runs as root before VS Code connects.
set -euo pipefail

WORKSPACE=/workspaces/sneezymud

# Docker creates volume mount points as root. Fix ownership so the non-root
# ubuntu user (remoteUser) can write to them.
chown ubuntu:ubuntu \
  /home/ubuntu \
  "${WORKSPACE}/build" \
  "${WORKSPACE}/.cache" \
  "${WORKSPACE}/lib/mutable"

# /run is ephemeral in containers - recreate the socket directory every boot.
mkdir -p /run/mysqld

# Seed the database on first boot. A sentinel file tracks whether seeding
# completed successfully, so partial failures are retried on next boot.
if ! [ -f /var/lib/mysql/.devcontainer-seeded ]; then
  echo "First boot - initializing and seeding database..."
  # mariadb-install-db is not idempotent - it fails if system tables already
  # exist. Skip it if a prior run already initialized the datadir.
  if ! [ -d /var/lib/mysql/mysql ]; then
    mariadb-install-db --user=root --datadir=/var/lib/mysql
  fi

  # Start MariaDB temporarily for seeding.
  mariadbd --user=root &
  for i in $(seq 1 30); do
    mariadb-admin ping --silent >/dev/null 2>&1 && break
    sleep 1
  done
  if ! mariadb-admin ping --silent >/dev/null 2>&1; then
    echo "MariaDB failed to start during seeding" >&2
    exit 1
  fi

  # CREATE USER IF NOT EXISTS no-ops if the user exists with a different auth
  # method. ALTER USER ensures unix_socket auth regardless of prior state.
  mariadb -e "
    CREATE DATABASE IF NOT EXISTS sneezy;
    CREATE DATABASE IF NOT EXISTS immortal;
    CREATE USER IF NOT EXISTS 'ubuntu'@'localhost' IDENTIFIED VIA unix_socket;
    ALTER USER 'ubuntu'@'localhost' IDENTIFIED VIA unix_socket;
    GRANT ALL ON *.* TO 'ubuntu'@'localhost';
    FLUSH PRIVILEGES;
  "

  for db in immortal sneezy; do
    for phase in tables views data; do
      dir="${WORKSPACE}/_Setup-data/sql_${phase}/${db}"
      [ -d "${dir}" ] || continue
      shopt -s nullglob
      for sql in "${dir}"/*.sql; do
        echo "  loading '${sql}'"
        mariadb "${db}" < "${sql}"
      done
      shopt -u nullglob
    done
  done

  mariadb-admin shutdown
  # shutdown returns before mariadbd fully exits - wait to avoid racing the
  # exec below, which would fail to acquire the InnoDB lock.
  wait
  touch /var/lib/mysql/.devcontainer-seeded
  echo "Database seeding complete."
fi

# Start MariaDB as PID 1. If MariaDB exits, the container stops.
exec mariadbd --user=root
