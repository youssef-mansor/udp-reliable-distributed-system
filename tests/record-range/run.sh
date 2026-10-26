#!/bin/bash
# compiles the recordRange tests against the real receiver.cpp and runs them
set -e
cd "$(dirname "$0")"
src=../../template_cpp/src
g++ -std=c++17 -Wall -Wextra -Werror -I"$src/include" test.cpp "$src/src/receiver.cpp" -o test_bin
./test_bin
