#!/bin/sh

set -eu
HOST="${HOST:-localhost}"
PORT="${PORT:-7900}"

mkdir -p output
OUTPUT="output/`basename $0 .sh`"

# Create character
(sleep 1
cat <<EOF
TestA
7estPw
c
TestA
 
yodel
rent
e
EOF
sleep 1
) | nc -c "$HOST" "$PORT" | tee -a "$OUTPUT"
grep -F "You long for the hills...you feel like Julie Andrews." "$OUTPUT" || (! echo "!!! Failed to yodel")
