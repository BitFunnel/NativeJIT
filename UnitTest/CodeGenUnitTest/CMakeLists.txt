# NativeJIT/UnitTest/CodeGenUnitTest

set(CPPFILES
  BitOperationsUnitTest.cpp
  CodeGenUnitTest.cpp
  FunctionBufferUnitTest.cpp
  InstructionEncodingUnitTest.cpp
  ML64Verifier.cpp
  stdafx.cpp
  )

set(PRIVATE_HFILES
  ML64Verifier.h
  stdafx.h
  targetver.h
  )

include_directories(${PROJECT_SOURCE_DIR}/src/CodeGen ${PROJECT_SOURCE_DIR}/UnitTest/Shared)

add_executable(CodeGenUnitTest ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (CodeGenUnitTest UnitTestShared CodeGen gtest gtest_main)

set_property(TARGET CodeGenUnitTest PROPERTY FOLDER "UnitTest")
