# NativeJIT/UnitTest/CodeGenUnitTest

set(CPPFILES
  CodeGenUnitTest.cpp
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

message("include_directories xxx: ${INCLUDE_DIRECTORIES}")

add_executable(CodeGenUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries (CodeGenUnitTest CodeGen gtest gtest_main)

set_property(TARGET CodeGenUnitTest PROPERTY FOLDER "UnitTest")
