# NativeJIT

cmake_minimum_required (VERSION 2.6)
project (NativeJIT)


###############################################################################
# From http://crascit.com/2015/07/25/cmake-gtest/

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in
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
  SET(VS_COMMON_CXX_FLAGS "/W4 /DUSING_GTEST")
  SET( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${VS_COMMON_CXX_FLAGS} /MTd" )
  SET( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${VS_COMMON_CXX_FLAGS} /MT" )
endif()

# This allows the solution to group projects into folders like src and UnitTest.
set_property( GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(inc)
add_subdirectory(src/CodeGen)
add_subdirectory(UnitTest/CodeGenUnitTest)

#add_custom_target(TOPLEVEL SOURCES ToDo.txt)
