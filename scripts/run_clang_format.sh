#! /bin/bash
#
# Finds all .cc and .h files in code/code/** and runs clang-format on
# them, editing them in-place using the rules in the .clang-format file
# in the root folder

find code/code/ -type f \( -iname "*.h" -o -iname "*.cc" \) -exec clang-format -i --verbose {} \;
