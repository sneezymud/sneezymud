#!/bin/bash
# Starts the game server in the background, waits for a successful boot, and
# writes the server PID to sneezy.pid. The caller is responsible for stopping
# the server when done.

set -euo pipefail

FOLLOW=false
if [[ "${1:-}" == "--follow" || "${1:-}" == "-f" ]]; then
  FOLLOW=true
fi

LOG=sneezy.log
PIDFILE=sneezy.pid

# Enable strict sanitizer options for testing.
# halt_on_error=1 ensures any undefined behavior aborts immediately.
export ASAN_OPTIONS="strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

./code/sneezy > "$LOG" 2>&1 &
SERVER_PID=$!
echo "$SERVER_PID" > "$PIDFILE"

if "$FOLLOW"; then
  tail -f "$LOG" &
  TAIL_PID=$!
  trap 'kill "$TAIL_PID" 2>/dev/null; wait "$TAIL_PID" 2>/dev/null' EXIT
fi

# Poll for the boot-complete message. Check process liveness each iteration so
# a sanitizer abort or crash fails immediately instead of burning the full
# timeout.
timeout 180 bash -c '
  while true; do
    if ! kill -0 '"$SERVER_PID"' 2>/dev/null; then
      echo "Server process died during boot" >&2
      tail -50 '"$LOG"' 2>/dev/null || true
      exit 1
    fi
    if grep -q "Entering game loop." '"$LOG"' 2>/dev/null; then
      break
    fi
    sleep 1
  done
'

if "$FOLLOW"; then
  # Boot complete - keep tailing until interrupted. Replace the shell with
  # tail so Ctrl+C cleanly stops it without leaving an orphan.
  trap - EXIT
  exec tail -f "$LOG" --pid="$SERVER_PID"
fi
