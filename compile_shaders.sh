# !/bin/bash

rm -r ./Executable/Shaders
mkdir ./Executable/Shaders
cp ./src/Shaders/* ./Executable/Shaders/


for f in ./Executable/Shaders/*.vert; do
    ./glslc $f -o $f.spv
    rm $f
done

for f in ./Executable/Shaders/*.frag; do
    ./glslc $f -o $f.spv
    rm $f
done

