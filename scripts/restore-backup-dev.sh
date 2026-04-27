#!/bin/bash
# Restores a nightly backup from sneezymud/backups GitHub Releases to a local
# dev environment. Supports both host-native MariaDB and Docker dev setups.
#
# Usage: restore-backup-dev.sh [--docker] [--date YYYY-MM-DD] [--yes] [--force]

set -euo pipefail

info()    { echo -e "\033[36m[INFO]\033[0m $1"; }
success() { echo -e "\033[32m[SUCCESS]\033[0m $1"; }
warning() { echo -e "\033[33m[WARNING]\033[0m $1"; }
error()   { echo -e "\033[31m[ERROR]\033[0m $1" >&2; exit 1; }

require_cmd() {
  command -v "$1" &>/dev/null || error "$1 is required but not found.${2:+\n  Install: $2}"
}

DOCKER_MODE=false
BACKUP_DATE=""
SKIP_CONFIRM=false
FORCE=false
TEMP_DIR=""
RESTART_CONTAINER=false
PRE_RESTORE_DUMP=""
BACKUP_REPO="sneezymud/backups"
SEED_MODE=false
SEED_DIR=""

# Docker container/volume names (stable across compose projects)
DB_CONTAINER="sneezy-db"
GAME_CONTAINER="sneezy"
DB_USER="sneezy"
DB_PASS="password"

# Run a mariadb command, routing through Docker or local socket as appropriate.
# Usage: db_cmd mariadb [args...]       - execute with args
#        db_cmd mariadb [args...] < file - pipe stdin (adds -i for docker exec)
db_cmd() {
  if $DOCKER_MODE; then
    local -a cmd=(docker exec)
    # If stdin is a pipe/file, docker exec needs -i to forward it
    [[ ! -t 0 ]] && cmd+=(-i)
    cmd+=("$DB_CONTAINER" "$1" -u "$DB_USER" -p"$DB_PASS")
    shift
    "${cmd[@]}" "$@"
  else
    "$@"
  fi
}

usage() {
  cat <<'EOF'
Usage: restore-backup-dev.sh [OPTIONS]

Restore a nightly backup to your local dev environment.

Options:
  --docker          Target Docker dev environment (sneezy-db container +
                    sneezy-mutable volume) instead of host-native MariaDB
  --date YYYY-MM-DD Restore a specific backup date (default: latest release)
  --seed            Restore from db/ seed files instead of a backup
                    release. Mutually exclusive with --date.
  --yes, -y         Skip confirmation prompt
  --force           Proceed even if the game server is running
  --help, -h        Show this help message

Examples:
  ./scripts/restore-backup-dev.sh                    # Latest backup, local DB
  ./scripts/restore-backup-dev.sh --date 2025-12-28  # Specific date
  ./scripts/restore-backup-dev.sh --docker            # Docker dev environment
  ./scripts/restore-backup-dev.sh --docker --yes      # Docker, no confirmation
  ./scripts/restore-backup-dev.sh --seed              # Seed data, local DB
  ./scripts/restore-backup-dev.sh --seed --docker     # Seed data, Docker
EOF
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --docker)    DOCKER_MODE=true; shift ;;
      --date)
        BACKUP_DATE="${2:-}"
        if [[ -z "$BACKUP_DATE" || "$BACKUP_DATE" == -* ]]; then
          error "--date requires a YYYY-MM-DD argument"
        fi
        if ! [[ "$BACKUP_DATE" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
          error "--date value '$BACKUP_DATE' does not match YYYY-MM-DD format"
        fi
        shift 2
        ;;
      --yes|-y)    SKIP_CONFIRM=true; shift ;;
      --force)     FORCE=true; shift ;;
      --help|-h)   usage; exit 0 ;;
      --seed)      SEED_MODE=true; shift ;;
      *)           error "Unknown option: $1. See --help for usage." ;;
    esac
  done

  if $SEED_MODE && [[ -n "$BACKUP_DATE" ]]; then
    error "--seed and --date are mutually exclusive"
  fi
}

cleanup() {
  if [[ -n "$TEMP_DIR" && -d "$TEMP_DIR" ]]; then
    rm -rf "$TEMP_DIR"
  fi
  if [[ -n "$PRE_RESTORE_DUMP" && -s "$PRE_RESTORE_DUMP" ]]; then
    warning "Pre-restore database dump preserved at: $PRE_RESTORE_DUMP"
    if $DOCKER_MODE; then
      warning "To recover: docker exec -i $DB_CONTAINER mariadb -u $DB_USER -p$DB_PASS < $PRE_RESTORE_DUMP"
    else
      warning "To recover: mariadb < $PRE_RESTORE_DUMP"
    fi
  fi
  if $DOCKER_MODE; then
    # Remove any lingering restore container (named with PID for uniqueness)
    docker rm -f "sneezy-restore-$$" 2>/dev/null || true
    # Restart game container if we stopped it, regardless of success/failure
    if $RESTART_CONTAINER; then
      docker start "$GAME_CONTAINER" 2>/dev/null || true
    fi
  fi
}

check_dependencies() {
  if ! $SEED_MODE; then
    require_cmd gh "https://github.com/cli/cli#installation"
    require_cmd tar
    require_cmd xz "sudo apt install xz-utils"
  fi
  if $DOCKER_MODE; then
    require_cmd docker
  else
    require_cmd mariadb
    require_cmd mariadb-dump
  fi

  if ! $DOCKER_MODE; then
    if ! mariadb -e 'SELECT 1' &>/dev/null; then
      error "Cannot connect to local MariaDB.\n  Is the server running? Check: systemctl status mariadb"
    fi
  fi

  if ! $SEED_MODE; then
    if ! gh auth status &>/dev/null; then
      error "GitHub CLI is not authenticated.\n  Run: gh auth login"
    fi

    if ! gh api "repos/${BACKUP_REPO}" &>/dev/null; then
      error "Cannot access ${BACKUP_REPO}.\n  Your GitHub account must be a member of the 'SneezyMUD hackers' team:\n  https://github.com/orgs/sneezymud/teams/hackers\n  Request access on the SneezyMUD Discord server."
    fi
  fi
}

check_server_running() {
  if $DOCKER_MODE; then
    if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "$GAME_CONTAINER"; then
      if $FORCE; then
        warning "Game container '$GAME_CONTAINER' is running. Stopping it for restore..."
        docker stop -t 30 "$GAME_CONTAINER" >/dev/null
        RESTART_CONTAINER=true
      else
        error "Game container '$GAME_CONTAINER' is running.\n  Stop it first, or use --force to stop and restart automatically."
      fi
    fi
    if ! docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "$DB_CONTAINER"; then
      error "Database container '$DB_CONTAINER' is not running.\n  Start it with: docker compose up -d db"
    fi
  else
    if pgrep -x sneezy &>/dev/null; then
      if $FORCE; then
        warning "Game server is running locally. Proceeding anyway (--force)."
      else
        error "Game server appears to be running (process named 'sneezy').\n  Stop it first, or use --force to proceed anyway."
      fi
    fi
  fi
}

download_backup() {
  TEMP_DIR=$(mktemp -d)

  # gh release download with no positional arg fetches the latest release.
  local -a dl_cmd=(gh release download)
  if [[ -n "$BACKUP_DATE" ]]; then
    dl_cmd+=("backup-${BACKUP_DATE}")
    info "Downloading backup for ${BACKUP_DATE}..."
  else
    info "Downloading latest backup..."
  fi

  if ! "${dl_cmd[@]}" --repo "$BACKUP_REPO" --pattern "*.tar.xz" \
      --dir "$TEMP_DIR"; then
    if [[ -n "$BACKUP_DATE" ]]; then
      error "Failed to download backup for ${BACKUP_DATE}.\n  Check that a release tagged 'backup-${BACKUP_DATE}' exists:\n  gh release list --repo ${BACKUP_REPO}"
    else
      error "Failed to download latest backup.\n  Check your access to ${BACKUP_REPO}:\n  gh release list --repo ${BACKUP_REPO}"
    fi
  fi

  local -a archives
  mapfile -t archives < <(find "$TEMP_DIR" -maxdepth 1 -name '*.tar.xz')
  if [[ ${#archives[@]} -eq 0 ]]; then
    error "No .tar.xz file found in downloaded release."
  elif [[ ${#archives[@]} -gt 1 ]]; then
    error "Multiple .tar.xz files in release - expected exactly one:\n  $(printf '  %s\n' "${archives[@]##*/}")"
  fi
  ARCHIVE="${archives[0]}"
  success "Downloaded $(basename "$ARCHIVE")"
}

extract_and_validate() {
  info "Extracting archive..."
  tar -xf "$ARCHIVE" -C "$TEMP_DIR"

  [[ -f "$TEMP_DIR/dbdump.sql" ]] || error "Archive does not contain dbdump.sql - is this a valid backup?"
  [[ -d "$TEMP_DIR/mutable" ]]    || error "Archive does not contain mutable/ - is this a valid backup?"
  MUTABLE_SRC="$TEMP_DIR/mutable"

  success "Archive validated (dbdump.sql + mutable/)"
}

validate_seed_data() {
  local repo_root
  repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
  SEED_DIR="$repo_root/db"
  [[ -d "$SEED_DIR" ]] || error "db/ not found at $SEED_DIR\n  Run this script from within the repository."
  success "Seed data found ($SEED_DIR)"
}

confirm() {
  $SKIP_CONFIRM && return

  local target="local host"
  $DOCKER_MODE && target="Docker ($DB_CONTAINER)"

  local source="backup"
  $SEED_MODE && source="seed data"

  echo
  warning "This will DROP and recreate the sneezy and immortal databases"
  warning "and replace the contents of the mutable/ directory."
  echo
  read -rp "Restore ${source} to ${target}? [y/N] " answer
  case "$answer" in
    [yY]|[yY][eE][sS]) ;;
    *) echo "Aborted."; exit 0 ;;
  esac
}

restore_database() {
  info "=== Restoring databases ==="

  # Snapshot current state for recovery if the import fails partway through.
  # Preserved on failure (cleanup trap warns about it), deleted on success.
  PRE_RESTORE_DUMP=$(mktemp "/tmp/sneezy-pre-restore-XXXXXX.sql")
  local dump_err
  dump_err=$(mktemp)
  info "Saving pre-restore database dump to $PRE_RESTORE_DUMP..."
  local dump_rc=0
  db_cmd mariadb-dump --lock-tables=false --databases sneezy immortal \
    > "$PRE_RESTORE_DUMP" 2>"$dump_err" || dump_rc=$?
  if [[ $dump_rc -eq 0 && -s "$PRE_RESTORE_DUMP" ]]; then
    success "Pre-restore snapshot saved"
  else
    warning "Could not snapshot existing databases (fresh setup?) - proceeding without recovery dump"
    [[ -s "$dump_err" ]] && warning "mariadb-dump: $(cat "$dump_err")"
    rm -f "$PRE_RESTORE_DUMP"
    PRE_RESTORE_DUMP=""
  fi
  rm -f "$dump_err"

  # The dump contains CREATE DATABASE statements, so we must DROP first.
  # Grants on db.* survive the DROP/recreate cycle.
  # Drop immortal first: it has cross-database FKs into sneezy.
  db_cmd mariadb -e "DROP DATABASE IF EXISTS immortal; DROP DATABASE IF EXISTS sneezy;"
  if $SEED_MODE; then
    db_cmd mariadb -e "CREATE DATABASE sneezy; CREATE DATABASE immortal;"
    local db dir sql
    for db in immortal sneezy; do
      dir="$SEED_DIR/${db}"
      [[ -d "$dir" ]] || continue
      for sql in "$dir"/*.sql; do
        [[ -f "$sql" ]] || continue
        info "  $db/$(basename "$sql")"
        db_cmd mariadb "$db" < "$sql"
      done
    done
  else
    db_cmd mariadb < "$TEMP_DIR/dbdump.sql"
  fi

  success "Databases restored (sneezy + immortal)"
}

restore_mutable() {
  info "=== Restoring mutable files ==="

  if $DOCKER_MODE; then
    # Find the mutable volume regardless of compose project prefix
    local -a volumes
    mapfile -t volumes < <(docker volume ls -q | grep 'sneezy-mutable$' || true)
    if [[ ${#volumes[@]} -eq 0 ]]; then
      error "Could not find a Docker volume matching '*sneezy-mutable'.\n  Create it by running: docker compose up -d"
    elif [[ ${#volumes[@]} -gt 1 ]]; then
      error "Multiple volumes match '*sneezy-mutable' - cannot disambiguate:\n  $(printf '  %s\n' "${volumes[@]}")\n  Remove or rename the extras so exactly one remains."
    fi
    local volume="${volumes[0]}"

    local restore_container="sneezy-restore-$$"

    # Spin up a lightweight container to manipulate the volume contents
    docker run -d --name "$restore_container" \
      -v "${volume}:/mnt/mutable" \
      alpine tail -f /dev/null >/dev/null

    docker exec "$restore_container" sh -c 'rm -rf /mnt/mutable/* /mnt/mutable/.[!.]*'

    if ! $SEED_MODE; then
      docker cp "$MUTABLE_SRC/." "$restore_container:/mnt/mutable/"
      # 1000:1000 matches the UID/GID the game server container runs as
      docker exec "$restore_container" chown -R 1000:1000 /mnt/mutable
    fi

    docker rm -f "$restore_container" >/dev/null
  else
    # Resolve to the repo root via the script's own location, not cwd
    local repo_root
    repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
    local target="$repo_root/lib/mutable"
    [[ -d "$target" ]] || error "Directory $target does not exist.\n  Expected the repo root at: $repo_root"

    rm -rf "${target:?}"/* "${target:?}"/.[!.]*

    if ! $SEED_MODE; then
      cp -a "$MUTABLE_SRC/." "$target/"
    fi
  fi

  success "Mutable files restored"
}

main() {
  parse_args "$@"
  trap cleanup EXIT
  check_dependencies
  check_server_running
  if $SEED_MODE; then
    validate_seed_data
  else
    download_backup
    extract_and_validate
  fi
  confirm
  restore_database
  restore_mutable

  # Restore succeeded - discard the pre-restore safety dump so cleanup
  # doesn't print recovery instructions
  rm -f "$PRE_RESTORE_DUMP"
  PRE_RESTORE_DUMP=""

  if $RESTART_CONTAINER; then
    info "Restarting game container '$GAME_CONTAINER'..."
    docker start "$GAME_CONTAINER" >/dev/null
    # Only clear the flag after a successful start so the cleanup trap can retry
    # on failure
    RESTART_CONTAINER=false
    success "Container restarted"
  fi

  echo
  if $SEED_MODE; then
    success "Seed restore complete!"
  else
    success "Backup restore complete!"
  fi
}

main "$@"
