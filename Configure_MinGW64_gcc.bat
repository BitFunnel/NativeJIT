setlocal

mkdir build-mingw64-gcc
pushd build-mingw64-gcc
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
popd
