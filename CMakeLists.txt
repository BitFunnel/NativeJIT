# NativeJIT

cmake_minimum_required(VERSION 2.6)
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
 
# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL )
 
include_directories(
  ${gtest_SOURCE_DIR}/include
)

###############################################################################


if(MSVC)
  SET( VS_COMMON_CXX_FLAGS "/W4 /WX" )
  SET( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${VS_COMMON_CXX_FLAGS} /MTd" )
  SET( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${VS_COMMON_CXX_FLAGS} /MT" )
else()
  SET( CLANG_COMMON_CXX_FLAGS "-msse4.2 -std=c++14")
  SET( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${CLANG_COMMON_CXX_FLAGS}" )
  SET( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${CLANG_COMMON_CXX_FLAGS}" )
endif()

# This allows the solution to group projects into folders like src and UnitTest.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(inc)
add_subdirectory(src/CodeGen)
#add_subdirectory(src/NativeJIT)
add_subdirectory(UnitTest/CodeGenUnitTest)
#add_subdirectory(UnitTest/NativeJITUnitTest)
add_subdirectory(UnitTest/Shared)

add_custom_target(TOPLEVEL SOURCES
  CMakeLists-googletest.txt.in
  Configure_OSX_Make.sh
  Configure_OSX_XCode.sh
  Configure_Windows.bat
  LICENSE.txt
  README.md
  ToDo.txt
)

enable_testing()
add_test(NAME CodeGenUnitTest COMMAND CodeGenUnitTest)
add_test(NAME NativeJITUnitTest COMMAND NativeJITUnitTest)
