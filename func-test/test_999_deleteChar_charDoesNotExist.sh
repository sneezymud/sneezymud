#!/bin/sh

set -eu
HOST="${HOST:-localhost}"
PORT="${PORT:-4000}"

mkdir -p output
OUTPUT="output/`basename $0 .sh`"

# Delete character

(sleep 1
cat <<EOF
TestA
7estPw
d
1
7estPw
EOF
sleep 1
) | nc -c "$HOST" "$PORT" | tee -a "$OUTPUT"
grep -F "Your account has been deleted, including all characters and equipment." "$OUTPUT" || (! echo "!!! Test failed, expected deletion didn't happen")

OUTPUT="$OUTPUT"_2

# Verify character doesn't exist anymore (can't log in)

(sleep 1
cat <<EOF
TestA
7estPw
EOF
sleep 1
) | nc -c "$HOST" "$PORT" | tee -a "$OUTPUT"
grep -F 'Incorrect login.' "$OUTPUT" || (! echo "!!! Test failed, account wasn't deleted")
