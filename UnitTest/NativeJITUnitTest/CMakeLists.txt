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

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT ${PROJECT_SOURCE_DIR}/UnitTest/Shared)

add_executable(NativeJITUnitTest ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (NativeJITUnitTest UnitTestShared NativeJIT CodeGen gtest gtest_main)

set_property(TARGET NativeJITUnitTest PROPERTY FOLDER "UnitTest")
