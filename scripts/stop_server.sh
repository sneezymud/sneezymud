#!/bin/bash
# Stops the game server started by start_server.sh and optionally dumps the log.
# Usage: stop_server.sh [--dump-log]

set -euo pipefail

PIDFILE=sneezy.pid
LOG=sneezy.log

if [ -f "$PIDFILE" ]; then
  kill "$(cat "$PIDFILE")" 2>/dev/null || true
  sleep 2
  kill -9 "$(cat "$PIDFILE")" 2>/dev/null || true
  rm -f "$PIDFILE"
fi

if [ "${1:-}" = "--dump-log" ] && [ -f "$LOG" ]; then
  echo "::group::${LOG}"
  cat "$LOG"
  echo "::endgroup::"
else
  rm -f "$LOG"
fi
