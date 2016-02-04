#!/bin/sh

echo "Configuring CMake for make."
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..

