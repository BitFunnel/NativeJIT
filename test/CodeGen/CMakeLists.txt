# NativeJIT/test/CodeGenTest

set(CPPFILES
  BitOperationsTest.cpp
  CodeGenTest.cpp
  FunctionBufferTest.cpp
  InstructionEncodingTest.cpp
  ML64Verifier.cpp
  )

set(PRIVATE_HFILES
  ML64Verifier.h
  )

include_directories(${PROJECT_SOURCE_DIR}/src/CodeGen ${PROJECT_SOURCE_DIR}/test/Shared)

add_executable(CodeGenTest ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (CodeGenTest NativeJITTestShared CodeGen gtest gtest_main)

set_property(TARGET CodeGenTest PROPERTY FOLDER "${NATIVEJIT_PREFIX}test")

add_test(NAME CodeGenTest COMMAND CodeGenTest)
