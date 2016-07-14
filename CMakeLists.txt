# NativeJIT

cmake_minimum_required(VERSION 2.6)

# Remove the warning that WIN32 is not defined in Cygwin any more.
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(NativeJIT_CMake)

###############################################################################
# From http://crascit.com/2015/07/25/cmake-gtest/

# Download and unpack googletest at configure time
configure_file(CMakeLists-googletest.txt.in
               googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

# Googletest will not compile under MinGW without this.
if(MINGW)
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
endif()

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)

include_directories(
  ${gtest_SOURCE_DIR}/include
)

###############################################################################

if (WIN32 OR CYGWIN)
  set(NATIVEJIT_PLATFORM_POSIX 0)
  set(NATIVEJIT_PLATFORM_WINDOWS 1)
else()
  set(NATIVEJIT_PLATFORM_POSIX 1)
  set(NATIVEJIT_PLATFORM_WINDOWS 0)
endif()

if(NATIVEJIT_PLATFORM_WINDOWS)
  # Note: -D works for both MSVC and GCC/Clang.
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_WINDOWS")
else()
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_POSIX")
endif()


if(MSVC)
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /W4 /WX")
  set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_FLAGS} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_CXX_FLAGS} /MT")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  # Need gnu++ instead of c++ so that GTest can access fdopen() etc.
  set(CMAKE_CXX_FLAGS "-march=native -std=gnu++14 -Wall -Wextra -Werror -Wold-style-cast")
else()
  set(CMAKE_CXX_FLAGS "-march=native -std=c++14 -Wall -Wextra -Werror -Wold-style-cast")
endif()


# This allows the solution to group projects into folders like src and UnitTest.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

include_directories(inc)
add_subdirectory(src/CodeGen)
add_subdirectory(src/NativeJIT)
add_subdirectory(UnitTest/CodeGenUnitTest)
add_subdirectory(UnitTest/NativeJITUnitTest)
add_subdirectory(UnitTest/Shared)
add_subdirectory(Examples)

add_custom_target(TOPLEVEL SOURCES
  CMakeLists-googletest.txt.in
  Configure_Make.bat
  Configure_Make.sh
  Configure_MSVC.bat
  LICENSE.txt
  README.md
)

set(ENV{GTEST_COLOR} 1)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
