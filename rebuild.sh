# !/bin/bash


rm -r Bin

cmake -H. -BBin -DCMAKE_BUILD_TYPE=Debug
cmake --build Bin

./post_compilation.sh
