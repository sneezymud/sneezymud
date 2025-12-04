#!/bin/sh
# Run sneezy with strict sanitizer options for development

export ASAN_OPTIONS="strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

exec ./code/sneezy "$@"
