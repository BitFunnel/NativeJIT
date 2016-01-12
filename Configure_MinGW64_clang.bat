setlocal

mkdir build-mingw64-clang
pushd build-mingw64-clang
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
popd
