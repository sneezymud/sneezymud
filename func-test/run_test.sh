#!/bin/sh

set -eu

rm -f output/*

for i in test_*.sh; do
	sh "$i" || (! echo "Failed the test suite at $i")
done
