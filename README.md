NativeJIT
====

Note: we're still in the early stages of open sourcing this code, which means that we
don't yet have proper documentation and reasonable starter examples.

NativeJIT is an open source, cross-platform library for high performance
just-in-time compilation of C++ expressions into X64 machine code.
NativeJIT was developed by the [Bing](http://www.bing.com) team at Microsoft and currently runs in production in the Bing search engine.

NativeJIT is published on GitHub at [TBD](about:blank)

Coming soon: online documentation at [bitfunnel.org](https://github.com/bitfunnel/nativejit).

Dependencies
------------

The NativeJIT repository uses Git submodules and should be cloned with the 
`--recursive` flag:

    git clone --recursive https://bitfunnel.visualstudio.com/DefaultCollection/_git/NativeJIT.Library

In order to build NativeJIT you will need CMake (3.3.2+), Pandoc (1.15.2+) and a C++11 compiler.

Following are specific instructions for building on various platforms.

### Linux (WORK IN PROGRESS - NOT TESTED)

NativeJIT can be built with Clang (3.4+) or GNU C++ (4.7+). We recommend the latest
version of Clang as it's much faster with template-heavy code like NativeJIT.

Run the following commands to install the minimal set of packages needed to 
build the core NativeJIT library on Ubuntu 14.04:

    sudo apt-get install \
        clang \
        cmake \
        pandoc 

In the root `NativeJIT` directory run:

    mkdir build
    cd build
    cmake ..
    make

The `build` directory is just an example. Any directory can be used for build 
destination.

Running the following command in the build directory will build and execute all 
the tests and examples:

    TODO

### OSX (WORK IN PROGRESS - NOT TESTED)

Install XCode and then run the following command to install required packages 
using Homebrew ([http://brew.sh/](http://brew.sh/)):

    brew install \
        cmake \
        pandoc

NativeJIT can be built on OS X using either standard \*nix makefiles or XCode.
In order to generate and build makefiles, in the root `NativeJIT` directory run:

    ./Configure_OSX_Make.sh
    
or run the following commands.

    mkdir build
    cd build
    cmake ..
    make

Alternatively you can generate XCode project by running

    ./Configure_OSX_XCode.sh

or by passing `-G Xcode` option to cmake:

    mkdir build
    cd build
    cmake -G Xcode ..

You can build and run unit tests by building the `check` target in XCode or by 
running make in the build directory:

    make --jobs 8 check

### Windows

Install the following tools:

- Visual Studio 2015
- CMake ([http://www.cmake.org/download/](http://www.cmake.org/download/))
- Pandoc ([http://pandoc.org/installing.html)](http://pandoc.org/installing.html))

In order to configure solution for Visual Studio 2015 run the following 
commands from the root `NativeJIT` directory:

    .\Configure_Windows.bat

This batch file does the following:

    mkdir build
    cd build
    cmake -G "Visual Studio 14 2015 Win64" ..

Instead of `cmake` you can also use `cmake-gui` and specify configuration 
settings in the UI. This configuration step has to be performed only once. From 
now on you can use the generated solution `build\BitFunnel.sln` from Visual Studio 
or build from command line using `cmake`:

    set PreferredToolArchitecture=x64
    cmake --build . --target
    cmake --build . --target INSTALL

In order to build and execute the unit tests and examples run:

    cmake --build . --target check -- /maxcpucount:8

Setting `PreferredToolArchitecture=x64` selects the 64-bit toolchain which 
dramatically improves build speed (Bond unit tests are too big to build with 
32-bit tools).
