# ! /bin/sh
# Verifies the game boots up completely and correctly

set -eu
timeout 180 ./code/sneezy 2>&1 | tee sneezy.log | (grep -q "Entering game loop." && killall sneezy) || (
  cat sneezy.log
  exit 1
)
