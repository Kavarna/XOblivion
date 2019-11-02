# !/bin/bash

rm -r ./Executable/
mkdir ./Executable/
cp -r ./Resources/ ./Executable/
cp ./Bin/XOblivion ./Executable/
./compile_shaders.sh

