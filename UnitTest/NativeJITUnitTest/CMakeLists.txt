# NativeJIT/UnitTest/NativeJITUnitTest

set(CPPFILES
  BitFunnelAcceptanceTest.cpp
  CastUnitTest.cpp
  ConditionalUnitTest.cpp
  ExpressionTreeUnitTest.cpp
  FloatingPointUnitTest.cpp
  FunctionUnitTest.cpp
  PackedUnitTest.cpp
  stdafx.cpp
  UnsignedUnitTest.cpp
)

set(PRIVATE_HFILES
  stdafx.h
  targetver.h
)

# CastUnitTest uses a lot of templates which creates a lot of sections in debug mode which requires the /bigobj or equivalent flag.
if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
elseif(CMAKE_COMPILER_IS_GNUCXX AND MINGW)
  # NOTE: the "-Wa, -mbig-obj" flags are supported by mingw
  # but not gcc in Linux.
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT ${PROJECT_SOURCE_DIR}/UnitTest/Shared)

add_executable(NativeJITUnitTest ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (NativeJITUnitTest UnitTestShared NativeJIT CodeGen gtest gtest_main)

set_property(TARGET NativeJITUnitTest PROPERTY FOLDER "UnitTest")
