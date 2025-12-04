#!/bin/sh
# Verifies the game boots up completely and correctly

set -eu

# Enable strict sanitizer options for testing
# halt_on_error=1 ensures any undefined behavior fails the test
export ASAN_OPTIONS="strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

timeout 180 ./code/sneezy 2>&1 | tee sneezy.log | (grep -q "Entering game loop." && killall sneezy) || (
  cat sneezy.log
  exit 1
)
