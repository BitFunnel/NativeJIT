# NativeJIT/UnitTest/CodeGenUnitTest

set(CPPFILES
  BitOperationsUnitTest.cpp
  CodeGenUnitTest.cpp
  FunctionBufferUnitTest.cpp
  ML64Verifier.cpp
  stdafx.cpp
  )

set(PRIVATE_HFILES
  ML64Verifier.h
  stdafx.h
  targetver.h
  )

set(PUBLIC_HFILES
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExecutionBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/FunctionBuffer.h
)

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

include_directories(${PROJECT_SOURCE_DIR}/src/CodeGen ${PROJECT_SOURCE_DIR}/UnitTest/Shared)

add_executable(CodeGenUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries (CodeGenUnitTest UnitTestShared CodeGen gtest gtest_main)

set_property(TARGET CodeGenUnitTest PROPERTY FOLDER "UnitTest")
