#!/bin/sh

echo "Configuring CMake for XCode."
mkdir build
cd build
cmake -G Xcode ..

