#!/usr/bin/env bash
# Install apt packages for CI workflows.
# Usage: install-packages.sh <package> [package ...]
set -euo pipefail
# apt-get install triggers a man-db cache rebuild (which takes a while and is
# useless for CI) via a dpkg trigger that checks for this flag file.
# Removing the file prevents the rebuild and speeds up the install.
sudo rm -f /var/lib/man-db/auto-update
sudo apt-get update
DEBIAN_FRONTEND=noninteractive TZ=utc sudo apt-get install --yes --no-install-recommends "$@"
