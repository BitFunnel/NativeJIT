NativeJIT
====

Note: we're still in the early stages of open sourcing this code, which means that we
don't yet have proper documentation and reasonable starter examples.

NativeJIT is an open source, cross-platform library for high performance
just-in-time compilation of C++ expressions into X64 machine code.
NativeJIT was developed by the [Bing](http://www.bing.com) team at Microsoft
and currently runs in production in the Bing search engine.

Coming soon: online documentation at [bitfunnel.org](https://github.com/bitfunnel/nativejit).
For now, [there's some preliminary documentation in `Documentation/`](https://github.com/BitFunnel/NativeJIT/tree/master/Documentation).

Dependencies
------------

In order to build NativeJIT you will need CMake (3.3.2+), and a modern C++
compiler (gcc 5+, clang 3.4+, or VC 2012+).

Following are specific instructions for building on various platforms.

### Linux

NativeJIT can be built with Clang (3.4+) or GNU C++ (5.0+). We recommend the
latest version of Clang as it's much faster with template-heavy code like
NativeJIT.

Run the following commands to install the minimal set of packages needed to 
build the core NativeJIT library on Ubuntu 15:

~~~
sudo apt-get install clang cmake
~~~

The current Ubuntu LTS version (14) points to an older version of cmake. To
install a new-enough CMake, see [this link](http://askubuntu.com/questions/610291/how-to-install-cmake-3-2-on-ubuntu-14-04).
If you're using gcc, you'll also need to make sure you have gcc-5 (`sudo apt-get install g++-5`).

In the root `NativeJIT` directory run:

~~~
./Configure_Linux_Make_clang.sh
cd build-linux-clang
make
~~~

### OSX

Install XCode and then run the following command to install required packages 
using Homebrew ([http://brew.sh/](http://brew.sh/)):

~~~
brew install cmake
~~~

NativeJIT can be built on OS X using either standard \*nix makefiles or XCode.
In order to generate and build makefiles, in the root `NativeJIT` directory run:

~~~
./Configure_OSX_Make.sh
~~~
    
Alternatively you can generate XCode project by running

~~~
./Configure_OSX_XCode.sh
~~~

You can build and run unit tests by building the `test` target in XCode or by
running make in the build directory:

~~~
make test
~~~

### Windows

Install the following tools:

- Visual Studio 2015 with C++ compiler
- CMake ([http://www.cmake.org/download/](http://www.cmake.org/download/))

You can get [the free version of Visual Studio here](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx).
Note that if you're installing Visual Studio for the first time and select the
default install options, you won't get a C++ compiler. To force the install of
the C++ compiler, you need to either create a new C++ project or open an
existing C++ project.

In order to configure solution for Visual Studio 2015 run the following 
commands from the root `NativeJIT` directory:

~~~
.\Configure_Windows_MSVC.bat
~~~

From 
now on you can use the generated solution `build-win-msvc\BitFunnel.sln` from Visual Studio
or build from command line using `cmake`:

