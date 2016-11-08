# NativeJIT

cmake_minimum_required(VERSION 2.6)

# Remove the warning that WIN32 is not defined in Cygwin any more.
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

# Project-wide settings for NativeJIT
project(NativeJIT CXX C)

###############################################################################

# gtest

add_subdirectory(googletest)

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


if (${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    message(STATUS "######################################## NativeJIT is root")
  else()
    message(STATUS "######################################## BitFunnel is root")
    set(NATIVEJIT_PREFIX "NativeJIT/")
endif (${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})

if(NATIVEJIT_PLATFORM_WINDOWS)
  # Note: -D works for both MSVC and GCC/Clang.
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_WINDOWS")
else()
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_POSIX")
endif()


set(GTEST_REQUIRED_FLAGS "-Wno-shift-sign-overflow -Wno-missing-noreturn -Wno-used-but-marked-unused -Wno-deprecated -Wno-missing-variable-declarations -Wno-global-constructors -Wno-weak-vtables")
set(WEVERYTHING_FLAGS "-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic ${GTEST_REQUIRED_FLAGS}")

if(MSVC)
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /W4 /WX")
  set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_FLAGS} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_CXX_FLAGS} /MT")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  # Need gnu++ instead of c++ so that GTest can access fdopen() etc.
  set(CMAKE_CXX_FLAGS "-march=native -std=gnu++14 -Wall -Wextra -Werror -Wold-style-cast -fstrict-aliasing -Wstrict-aliasing")
else()
  # TODO: define a target for -Weverything.
  # set(CMAKE_CXX_FLAGS "-msse4.2 -std=c++14 -Wall -Wextra -Werror -Wold-style-cast ${WEVERYTHING_FLAGS}")
  set(CMAKE_CXX_FLAGS "-march=native -std=c++14 -Wall -Wextra -Werror -Wold-style-cast")
endif()


# This allows the solution to group projects into folders like src and test.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

include_directories(inc)
add_subdirectory(src/CodeGen)
add_subdirectory(src/NativeJIT)
add_subdirectory(test/CodeGen)
add_subdirectory(test/NativeJIT)
add_subdirectory(test/Shared)
add_subdirectory(Examples)

add_custom_target(TOPLEVEL_NATIVEJIT SOURCES
  Configure_Make.bat
  Configure_Make.sh
  Configure_MSVC.bat
  LICENSE.txt
  README.md
)

set(ENV{GTEST_COLOR} 1)
# add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
