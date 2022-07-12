#!/bin/sh

set -eu
HOST="${HOST:-localhost}"
PORT="${PORT:-4000}"

mkdir -p output
OUTPUT="output/`basename $0 .sh`"

# Create character
(sleep 1
cat <<EOF
NEW
TestA
7estPw
7estPw
a
TestA
EOF
sleep 0.5
echo ' '
sleep 0.5
echo ' '
sleep 0.5
echo ' '
sleep 0.5
cat <<EOF
y
 
d
 
EOF
sleep 0.5
echo ' '
cat <<EOF
 
d
rent
e
EOF
sleep 1
echo ' '
) | nc -c "$HOST" "$PORT" | tee -a "$OUTPUT"
# Pheeeew, that was long. We should streamline the character creation process.
grep -F "Rosemary tells you, 'Have a nice stay!" "$OUTPUT" || (! echo "!!! Failed to create character")

OUTPUT="$OUTPUT"_2

# Verify character exists (ie. it can log in)

(sleep 1
cat <<EOF
TestA
7estPw
c
TestA
  
rent
e
EOF
sleep 1
) | nc -c "$HOST" "$PORT" | tee -a "$OUTPUT"
grep -F "Rosemary stores your stuff in the safe, and shows you to your room." "$OUTPUT" || (! echo "!!! Failed to log in after creation")
