# NativeJIT/UnitTest/NativeJITUnitTest

set(CPPFILES
  BitFunnelAcceptanceTest.cpp
  BitOperationsUnitTest.cpp
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

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT)

add_executable(NativeJITUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries (NativeJITUnitTest NativeJIT CodeGen gtest gtest_main)

set_property(TARGET NativeJITUnitTest PROPERTY FOLDER "UnitTest")
