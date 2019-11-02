# !/bin/bash

cmake -H. -BBin
cmake --build Bin
./post_compilation.sh
